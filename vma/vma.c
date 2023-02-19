// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <gfp.h>
#include <vma.h>
#include <page.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <limits.h>
#include <rbtree.h>
#include <pgtable.h>

/*
 * This linked list is used in pair with free_vmap_area_root.
 * It gives O(1) access to prev/next to perform fast coalescing.
 */
static LIST_HEAD(free_vmap_area_list);

/*
 * This augment red-black tree represents the free vmap space.
 * All vmap_area objects in this tree are sorted by va->va_start
 * address. It is used for allocation and merging when a vmap
 * object is released.
 *
 * Each vmap_area node contains a maximum available free block
 * of its sub-tree, right or left. Therefore it is possible to
 * find a lowest match of free area.
 */
static struct rb_root free_vmap_area_root = RB_ROOT;

static LIST_HEAD(vmap_area_list);
static struct rb_root vmap_area_root = RB_ROOT;

static bool vmap_initialized;

/*
 * This kmem_cache is used for vmap_area objects. Instead of
 * allocating from slab we reuse an object from this cache to
 * make things faster. Especially in "no edge" splitting of
 * free block.
 */
static struct kmem_cache *vmap_area_cachep;

static __always_inline unsigned long
va_size(struct vmap_area *va)
{
    return (va->va_end - va->va_start);
}

RB_DECLARE_CALLBACKS_MAX(static, free_vmap_area_rb_augment_cb,
                         struct vmap_area, rb_node,
                         unsigned long, subtree_max_size, va_size)

static __always_inline unsigned long
get_subtree_max_size(struct rb_node *node)
{
    struct vmap_area *va;
    va = rb_entry_safe(node, struct vmap_area, rb_node);
    return va ? va->subtree_max_size : 0;
}

static __always_inline bool
is_within_this_va(struct vmap_area *va, unsigned long size,
                  unsigned long align, unsigned long vstart)
{
    unsigned long nva_start_addr;

    if (va->va_start > vstart)
        nva_start_addr = ALIGN(va->va_start, align);
    else
        nva_start_addr = ALIGN(vstart, align);

    /* Can be overflowed due to big size or alignment. */
    if (nva_start_addr + size < nva_start_addr ||
        nva_start_addr < vstart)
        return false;

    return (nva_start_addr + size <= va->va_end);
}

/*
 * Find the first free block(lowest start address) in the tree,
 * that will accomplish the request corresponding to passing
 * parameters.
 */
static __always_inline struct vmap_area *
find_vmap_lowest_match(unsigned long size,
                       unsigned long align,
                       unsigned long vstart)
{
    struct vmap_area *va;
    struct rb_node *node;
    unsigned long length;

    /* Start from the root. */
    node = free_vmap_area_root.rb_node;

    /* Adjust the search size for alignment overhead. */
    length = size + align - 1;

    while (true) {
        va = rb_entry(node, struct vmap_area, rb_node);

        if (get_subtree_max_size(node->rb_left) >= length &&
            vstart < va->va_start) {
            node = node->rb_left;
        } else {
            if (is_within_this_va(va, size, align, vstart))
                return va;

            /*
             * Does not make sense to go deeper towards the right
             * sub-tree if it does not have a free block that is
             * equal or bigger to the requested search length.
             */
            if (get_subtree_max_size(node->rb_right) >= length) {
                node = node->rb_right;
                continue;
            }

            /*
             * OK. We roll back and find the first right sub-tree,
             * that will satisfy the search criteria. It can happen
             * only once due to "vstart" restriction.
             */
            while ((node = rb_parent(node))) {
                va = rb_entry(node, struct vmap_area, rb_node);
                if (is_within_this_va(va, size, align, vstart))
                    return va;

                if (get_subtree_max_size(node->rb_right) >= length &&
                    vstart <= va->va_start) {
                    node = node->rb_right;
                    break;
                }
            }
        }
    }

    return NULL;
}

enum fit_type {
    NOTHING_FIT = 0,
    FL_FIT_TYPE = 1,    /* full fit */
    LE_FIT_TYPE = 2,    /* left edge fit */
    RE_FIT_TYPE = 3,    /* right edge fit */
    NE_FIT_TYPE = 4     /* no edge fit */
};

static __always_inline enum fit_type
classify_va_fit_type(struct vmap_area *va,
                     unsigned long nva_start_addr,
                     unsigned long size)
{
    enum fit_type type;

    /* Check if it is within VA. */
    if (nva_start_addr < va->va_start ||
        nva_start_addr + size > va->va_end)
        return NOTHING_FIT;

    /* Now classify. */
    if (va->va_start == nva_start_addr) {
        if (va->va_end == nva_start_addr + size)
            type = FL_FIT_TYPE;
        else
            type = LE_FIT_TYPE;
    } else if (va->va_end == nva_start_addr + size) {
        type = RE_FIT_TYPE;
    } else {
        type = NE_FIT_TYPE;
    }

    return type;
}

static __always_inline void
unlink_va(struct vmap_area *va, struct rb_root *root)
{
    BUG_ON(RB_EMPTY_NODE(&va->rb_node));

    if (root == &free_vmap_area_root)
        rb_erase_augmented(&va->rb_node, root, &free_vmap_area_rb_augment_cb);
    else
        rb_erase(&va->rb_node, root);

    list_del(&va->list);
    RB_CLEAR_NODE(&va->rb_node);
}

static __always_inline void
augment_tree_propagate_from(struct vmap_area *va)
{
    /*
     * Populate the tree from bottom towards the root until
     * the calculated maximum available size of checked node
     * is equal to its current one.
     */
    free_vmap_area_rb_augment_cb_propagate(&va->rb_node, NULL);
}

static __always_inline struct rb_node **
find_va_links(struct vmap_area *va,
              struct rb_root *root,
              struct rb_node *from,
              struct rb_node **parent)
{
    struct rb_node **link;
    struct vmap_area *tmp_va;

    if (root) {
        link = &root->rb_node;
        if (unlikely(!*link)) {
            *parent = NULL;
            return link;
        }
    } else {
        link = &from;
    }

    do {
        tmp_va = rb_entry(*link, struct vmap_area, rb_node);

        if (va->va_start < tmp_va->va_end &&
            va->va_end <= tmp_va->va_start) {
            link = &(*link)->rb_left;
        } else if (va->va_end > tmp_va->va_start &&
                   va->va_start >= tmp_va->va_end) {
            link = &(*link)->rb_right;
        } else {
            panic("vmalloc bug: 0x%lx-0x%lx overlaps with 0x%lx-0x%lx",
                  va->va_start, va->va_end, tmp_va->va_start, tmp_va->va_end);

            return NULL;
        }
    } while (*link);

    *parent = &tmp_va->rb_node;
    return link;
}

static __always_inline void
link_va(struct vmap_area *va,
        struct rb_root *root,
        struct rb_node *parent,
        struct rb_node **link,
        struct list_head *head)
{
    /*
     * VA is still not in the list, but we can
     * identify its future previous list_head node.
     */
    if (likely(parent)) {
        head = &rb_entry(parent, struct vmap_area, rb_node)->list;
        if (&parent->rb_right != link)
            head = head->prev;
    }

    /* Insert to the rb-tree */
    rb_link_node(&va->rb_node, parent, link);
    if (root == &free_vmap_area_root) {
        /*
         * Some explanation here. Just perform simple insertion
         * to the tree. We do not set va->subtree_max_size to
         * its current size before calling rb_insert_augmented().
         * It is because of we populate the tree from the bottom
         * to parent levels when the node _is_ in the tree.
         *
         * Therefore we set subtree_max_size to zero after insertion,
         * to let __augment_tree_propagate_from() puts everything to
         * the correct order later on.
         */
        rb_insert_augmented(&va->rb_node, root, &free_vmap_area_rb_augment_cb);
        va->subtree_max_size = 0;
    } else {
        rb_insert_color(&va->rb_node, root);
    }

    /* Address-sort this list */
    list_add(&va->list, head);
}

static void
insert_vmap_area_augment(struct vmap_area *va,
                         struct rb_node *from,
                         struct rb_root *root,
                         struct list_head *head)
{
    struct rb_node **link;
    struct rb_node *parent;

    if (from)
        link = find_va_links(va, NULL, from, &parent);
    else
        link = find_va_links(va, root, NULL, &parent);

    if (link) {
        link_va(va, root, parent, link, head);
        augment_tree_propagate_from(va);
    }
}

static __always_inline int
adjust_va_to_fit_type(struct vmap_area *va,
                      unsigned long nva_start_addr,
                      unsigned long size,
                      enum fit_type type)
{
    struct vmap_area *lva = NULL;

    if (type == FL_FIT_TYPE) {
        /*
         * No need to split VA, it fully fits.
         *
         * |               |
         * V      NVA      V
         * |---------------|
         */
        unlink_va(va, &free_vmap_area_root);
        kmem_cache_free(vmap_area_cachep, va);
    } else if (type == LE_FIT_TYPE) {
        /*
         * Split left edge of fit VA.
         *
         * |       |
         * V  NVA  V   R
         * |-------|-------|
         */
        va->va_start += size;
    } else if (type == RE_FIT_TYPE) {
        /*
         * Split right edge of fit VA.
         *
         *         |       |
         *     L   V  NVA  V
         * |-------|-------|
         */
        va->va_end = nva_start_addr;
    } else if (type == NE_FIT_TYPE) {
        /*
         * Split no edge of fit VA.
         *
         *     |       |
         *   L V  NVA  V R
         * |---|-------|---|
         */
        lva = kmem_cache_alloc(vmap_area_cachep, GFP_NOWAIT);
        if (!lva)
            return -1;

        /*
         * Build the remainder.
         */
        lva->va_start = va->va_start;
        lva->va_end = nva_start_addr;

        /*
         * Shrink this VA to remaining size.
         */
        va->va_start = nva_start_addr + size;
    } else {
        return -1;
    }

    if (type != FL_FIT_TYPE) {
        augment_tree_propagate_from(va);

        if (lva)    /* type == NE_FIT_TYPE */
            insert_vmap_area_augment(lva,
                                     &va->rb_node,
                                     &free_vmap_area_root,
                                     &free_vmap_area_list);
    }

    return 0;
}

static __always_inline unsigned long
__alloc_vmap_area(unsigned long size, unsigned long align,
                  unsigned long vstart, unsigned long vend)
{
    enum fit_type type;
    struct vmap_area *va;
    unsigned long nva_start_addr;

    va = find_vmap_lowest_match(size, align, vstart);
    if (unlikely(!va))
        return vend;

    if (va->va_start > vstart)
        nva_start_addr = ALIGN(va->va_start, align);
    else
        nva_start_addr = ALIGN(vstart, align);

    /* Check the "vend" restriction. */
    if (nva_start_addr + size > vend)
        return vend;

    /* Classify what we have found. */
    type = classify_va_fit_type(va, nva_start_addr, size);
    BUG_ON(type == NOTHING_FIT);

    /* Update the free vmap_area. */
    if (adjust_va_to_fit_type(va, nva_start_addr, size, type))
        return vend;

    return nva_start_addr;
}

static void
insert_vmap_area(struct vmap_area *va,
                 struct rb_root *root,
                 struct list_head *head)
{
    struct rb_node **link;
    struct rb_node *parent;

    link = find_va_links(va, root, NULL, &parent);
    if (link)
        link_va(va, root, parent, link, head);
}

/*
 * Allocate a region of KVA of the specified size and alignment, within the
 * vstart and vend.
 */
static struct vmap_area *
alloc_vmap_area(unsigned long size, unsigned long align,
                unsigned long vstart, unsigned long vend,
                gfp_t gfp_mask)
{
    unsigned long addr;
    struct vmap_area *va;

    if (unlikely(!vmap_initialized))
        return ERR_PTR(-EBUSY);

    gfp_mask = gfp_mask & GFP_RECLAIM_MASK;

    va = kmem_cache_alloc(vmap_area_cachep, gfp_mask);
    if (unlikely(!va))
        return ERR_PTR(-ENOMEM);

    /*
     * If an allocation fails, the "vend" address is
     * returned. Therefore trigger the overflow path.
     */
    addr = __alloc_vmap_area(size, align, vstart, vend);
    if (unlikely(addr == vend))
        panic("overflow!");

    va->va_start = addr;
    va->va_end = addr + size;
    va->vm = NULL;

    insert_vmap_area(va, &vmap_area_root, &vmap_area_list);

    BUG_ON(!IS_ALIGNED(va->va_start, align));
    BUG_ON(va->va_start < vstart);
    BUG_ON(va->va_end > vend);

    return va;
}

static inline void
setup_vmalloc_vm_locked(struct vm_struct *vm,
                        struct vmap_area *va,
                        unsigned long flags)
{
    vm->flags = flags;
    vm->addr = (void *)va->va_start;
    vm->size = va->va_end - va->va_start;
    va->vm = vm;
}

static void
setup_vmalloc_vm(struct vm_struct *vm,
                 struct vmap_area *va,
                 unsigned long flags)
{
    setup_vmalloc_vm_locked(vm, va, flags);
}

static struct vm_struct *
__get_vm_area_node(unsigned long size,
                   unsigned long align,
                   unsigned long flags,
                   unsigned long start,
                   unsigned long end,
                   gfp_t gfp_mask)
{
    struct vm_struct *area;
    struct vmap_area *va;

    size = PAGE_ALIGN(size);
    if (unlikely(!size))
        return NULL;

    if (flags & VM_IOREMAP)
        align = 1ul << clamp_t(int, get_count_order_long(size),
                               PAGE_SHIFT, IOREMAP_MAX_ORDER);

    area = kzalloc(sizeof(*area), gfp_mask & GFP_RECLAIM_MASK);
    if (unlikely(!area))
        return NULL;

    va = alloc_vmap_area(size, align, start, end, gfp_mask);
    if (IS_ERR(va)) {
        kfree(area);
        return NULL;
    }

    setup_vmalloc_vm(area, va, flags);
    return area;
}

/**
 * get_vm_area - reserve a contiguous kernel virtual area
 * @size:    size of the area
 * @flags:   %VM_IOREMAP for I/O mappings or VM_ALLOC
 *
 * Search an area of @size in the kernel virtual mapping area,
 * and reserved it for out purposes.  Returns the area descriptor
 * on success or %NULL on failure.
 *
 * Return: the area descriptor on success or %NULL on failure.
 */
struct vm_struct *
get_vm_area(unsigned long size, unsigned long flags)
{
    return __get_vm_area_node(size, 1, flags,
                              VMALLOC_START, VMALLOC_END,
                              GFP_KERNEL);
}
EXPORT_SYMBOL(get_vm_area);

static void
vmap_init_free_space(void)
{
    struct vmap_area *free;

    free = kmem_cache_zalloc(vmap_area_cachep, GFP_NOWAIT);
    if (!free)
        panic("bad cache for vmap_area!");

    free->va_start = 1;
    free->va_end = ULONG_MAX;

    insert_vmap_area_augment(free, NULL,
                             &free_vmap_area_root,
                             &free_vmap_area_list);
}

void
free_vm_area(struct vm_struct *area)
{
    /* Todo: */
}
EXPORT_SYMBOL(free_vm_area);

int
init_module(void)
{
    printk("module[vma]: init begin ...\n");

    /*
     * Create the cache for vmap_area objects.
     */
    vmap_area_cachep = KMEM_CACHE(vmap_area, SLAB_PANIC);

    /*
     * Now we can initialize a free vmap space.
     */
    vmap_init_free_space();
    vmap_initialized = true;

    printk("module[vma]: init end!\n");
    return 0;
}

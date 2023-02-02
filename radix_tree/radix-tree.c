// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <errno.h>
#include <export.h>
#include <printk.h>
#include <string.h>
#include <radix-tree.h>

/*
 * Radix tree node cache.
 */
struct kmem_cache *radix_tree_node_cachep;

static inline struct radix_tree_node *entry_to_node(void *ptr)
{
    return (void *)((unsigned long)ptr & ~RADIX_TREE_INTERNAL_NODE);
}

/*
 * The maximum index which can be stored in a radix tree
 */
static inline unsigned long shift_maxindex(unsigned int shift)
{
    return (RADIX_TREE_MAP_SIZE << shift) - 1;
}

static inline unsigned long node_maxindex(const struct radix_tree_node *node)
{
    return shift_maxindex(node->shift);
}

static unsigned
radix_tree_load_root(const struct radix_tree_root *root,
                     struct radix_tree_node **nodep,
                     unsigned long *maxindex)
{
    struct radix_tree_node *node = root->xa_head;

    *nodep = node;

    if (likely(radix_tree_is_internal_node(node))) {
        node = entry_to_node(node);
        *maxindex = node_maxindex(node);
        return node->shift + RADIX_TREE_MAP_SHIFT;
    }

    *maxindex = 0;
    return 0;
}

static unsigned int
radix_tree_descend(const struct radix_tree_node *parent,
                   struct radix_tree_node **nodep, unsigned long index)
{
    unsigned int offset = (index >> parent->shift) & RADIX_TREE_MAP_MASK;
    void **entry = parent->slots[offset];

    *nodep = (void *)entry;
    return offset;
}

void *
__radix_tree_lookup(const struct radix_tree_root *root,
                    unsigned long index,
                    struct radix_tree_node **nodep,
                    void ***slotp)
{
    void **slot;
    unsigned long maxindex;
    struct radix_tree_node *node, *parent;

    parent = NULL;
    slot = (void **)&root->xa_head;
    radix_tree_load_root(root, &node, &maxindex);
    if (index > maxindex)
        return NULL;

    while (radix_tree_is_internal_node(node)) {
        unsigned offset;

        parent = entry_to_node(node);
        offset = radix_tree_descend(parent, &node, index);
        slot = parent->slots + offset;
        if (parent->shift == 0)
            break;
    }

    if (nodep)
        *nodep = parent;
    if (slotp)
        *slotp = slot;
    return node;
}

void *
radix_tree_lookup(const struct radix_tree_root *root,
                  unsigned long index)
{
    return __radix_tree_lookup(root, index, NULL, NULL);
}
EXPORT_SYMBOL(radix_tree_lookup);

static int
radix_tree_extend(struct radix_tree_root *root,
                  unsigned long index, unsigned int shift)
{
    void *entry;
    unsigned int maxshift;

    /* Figure out what the shift should be.  */
    maxshift = shift;
    while (index > shift_maxindex(maxshift))
        maxshift += RADIX_TREE_MAP_SHIFT;

    entry = root->xa_head;
    if (!entry)
        return maxshift + RADIX_TREE_MAP_SHIFT;

    panic("%s: (%u, %u, %u)!", __func__, index, shift);
}

static struct radix_tree_node *
radix_tree_node_alloc(gfp_t gfp_mask, struct radix_tree_node *parent,
                      struct radix_tree_root *root,
                      unsigned int shift, unsigned int offset,
                      unsigned int count, unsigned int nr_values)
{
    struct radix_tree_node *ret = NULL;

    ret = kmem_cache_alloc(radix_tree_node_cachep, gfp_mask);
    BUG_ON(radix_tree_is_internal_node(ret));
    if (ret) {
        ret->shift = shift;
        ret->offset = offset;
        ret->parent = parent;
        ret->array = root;
    }
    return ret;
}

static inline void *node_to_entry(void *ptr)
{
    return (void *)((unsigned long)ptr | RADIX_TREE_INTERNAL_NODE);
}

static int
__radix_tree_create(struct radix_tree_root *root,
                    unsigned long index,
                    struct radix_tree_node **nodep,
                    void ***slotp)
{
    unsigned long maxindex;
    unsigned long max = index;
    struct radix_tree_node *child;
    gfp_t gfp = GFP_KERNEL;
    unsigned int shift, offset = 0;
    struct radix_tree_node *node = NULL;
    void **slot = (void **)&root->xa_head;

    shift = radix_tree_load_root(root, &child, &maxindex);

    /* Make sure the tree is high enough.  */
    if (max > maxindex) {
        int error = radix_tree_extend(root, max, shift);
        if (error < 0)
            return error;
        shift = error;
        child = root->xa_head;
    }

    while (shift > 0) {
        shift -= RADIX_TREE_MAP_SHIFT;
        if (child == NULL) {
            /* Have to add a child node.  */
            child = radix_tree_node_alloc(gfp, node, root, shift,
                                          offset, 0, 0);
            if (!child)
                return -ENOMEM;
            *slot = node_to_entry(child);
        } else if (!radix_tree_is_internal_node(child))
            break;

        /* Go a level down */
        node = entry_to_node(child);
        offset = radix_tree_descend(node, &child, index);
        slot = &node->slots[offset];
    }

    if (nodep)
        *nodep = node;
    if (slotp)
        *slotp = slot;
    return 0;
}

static inline int
insert_entries(struct radix_tree_node *node,
               void **slot, void *item, bool replace)
{
    if (*slot)
        return -EEXIST;
    *slot = item;
    return 1;
}

int
radix_tree_insert(struct radix_tree_root *root,
                  unsigned long index, void *item)
{
    int error;
    void **slot;
    struct radix_tree_node *node;

    BUG_ON(radix_tree_is_internal_node(item));

    error = __radix_tree_create(root, index, &node, &slot);
    if (error)
        return error;

    error = insert_entries(node, slot, item, false);
    if (error < 0)
        return error;

    return 0;
}
EXPORT_SYMBOL(radix_tree_insert);

static void
radix_tree_node_ctor(void *arg)
{
    struct radix_tree_node *node = arg;
    memset(node, 0, sizeof(*node));
}

void radix_tree_init(void)
{
    BUG_ON(XA_CHUNK_SIZE > 255);
    radix_tree_node_cachep =
        kmem_cache_create("radix_tree_node",
                          sizeof(struct radix_tree_node), 0,
                          SLAB_PANIC | SLAB_RECLAIM_ACCOUNT,
                          radix_tree_node_ctor);
}

static int
init_module(void)
{
    printk("module[radix-tree]: init begin ...\n");

    radix_tree_init();

    printk("module[radix-tree]: init end!\n");
    return 0;
}

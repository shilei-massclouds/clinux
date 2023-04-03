// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <fork.h>
#include <mman.h>
#include <stat.h>
#include <errno.h>
#include <export.h>
#include <limits.h>
#include <rbtree.h>
#include <current.h>

/* enforced gap between the expanding stack and other mappings. */
unsigned long stack_guard_gap = 256UL<<PAGE_SHIFT;
EXPORT_SYMBOL(stack_guard_gap);

static int
find_vma_links(struct mm_struct *mm, unsigned long addr,
               unsigned long end, struct vm_area_struct **pprev,
               struct rb_node ***rb_link, struct rb_node **rb_parent)
{
    struct rb_node **__rb_link, *__rb_parent, *rb_prev;

    __rb_link = &mm->mm_rb.rb_node;
    rb_prev = __rb_parent = NULL;

    while (*__rb_link) {
        struct vm_area_struct *vma_tmp;

        __rb_parent = *__rb_link;
        vma_tmp = rb_entry(__rb_parent, struct vm_area_struct, vm_rb);

        if (vma_tmp->vm_end > addr) {
            /* Fail if an existing vma overlaps the area */
            if (vma_tmp->vm_start < end)
                return -ENOMEM;
            __rb_link = &__rb_parent->rb_left;
        } else {
            rb_prev = __rb_parent;
            __rb_link = &__rb_parent->rb_right;
        }
    }

    *pprev = NULL;
    if (rb_prev)
        *pprev = rb_entry(rb_prev, struct vm_area_struct, vm_rb);
    *rb_link = __rb_link;
    *rb_parent = __rb_parent;
    return 0;
}

static inline unsigned long vma_compute_gap(struct vm_area_struct *vma)
{
    unsigned long gap, prev_end;

    /*
     * Note: in the rare case of a VM_GROWSDOWN above a VM_GROWSUP, we
     * allow two stack_guard_gaps between them here, and when choosing
     * an unmapped area; whereas when expanding we only require one.
     * That's a little inconsistent, but keeps the code here simpler.
     */
    gap = vm_start_gap(vma);
    if (vma->vm_prev) {
        prev_end = vm_end_gap(vma->vm_prev);
        if (gap > prev_end)
            gap -= prev_end;
        else
            gap = 0;
    }
    return gap;
}

RB_DECLARE_CALLBACKS_MAX(static, vma_gap_callbacks,
                         struct vm_area_struct, vm_rb,
                         unsigned long, rb_subtree_gap, vma_compute_gap)

/*
 * Update augmented rbtree rb_subtree_gap values after vma->vm_start or
 * vma->vm_prev->vm_end values changed, without modifying the vma's position
 * in the rbtree.
 */
static void vma_gap_update(struct vm_area_struct *vma)
{
    /*
     * As it turns out, RB_DECLARE_CALLBACKS_MAX() already created
     * a callback function that does exactly what we want.
     */
    vma_gap_callbacks_propagate(&vma->vm_rb, NULL);
}

static inline void
vma_rb_insert(struct vm_area_struct *vma, struct rb_root *root)
{
    /* All rb_subtree_gap values must be consistent prior to insertion */
    rb_insert_augmented(&vma->vm_rb, root, &vma_gap_callbacks);
}

void __vma_link_rb(struct mm_struct *mm, struct vm_area_struct *vma,
        struct rb_node **rb_link, struct rb_node *rb_parent)
{
    /* Update tracking information for the gap following the new vma. */
    if (vma->vm_next)
        vma_gap_update(vma->vm_next);
    else
        mm->highest_vm_end = vm_end_gap(vma);

    /*
     * vma->vm_prev wasn't known when we followed the rbtree to find the
     * correct insertion point for that vma. As a result, we could not
     * update the vma vm_rb parents rb_subtree_gap values on the way down.
     * So, we first insert the vma with a zero rb_subtree_gap value
     * (to be consistent with what we did on the way down), and then
     * immediately update the gap to the correct value. Finally we
     * rebalance the rbtree after all augmented values have been set.
     */
    rb_link_node(&vma->vm_rb, rb_parent, rb_link);
    vma->rb_subtree_gap = 0;
    vma_gap_update(vma);
    vma_rb_insert(vma, &mm->mm_rb);
}

static void
__vma_link(struct mm_struct *mm, struct vm_area_struct *vma,
    struct vm_area_struct *prev, struct rb_node **rb_link,
    struct rb_node *rb_parent)
{
    __vma_link_list(mm, vma, prev);
    __vma_link_rb(mm, vma, rb_link, rb_parent);
}

static void
vma_link(struct mm_struct *mm, struct vm_area_struct *vma,
         struct vm_area_struct *prev, struct rb_node **rb_link,
         struct rb_node *rb_parent)
{
    __vma_link(mm, vma, prev, rb_link, rb_parent);
}

/* Insert vm structure into process list sorted by address
 * and into the inode's i_mmap tree.  If vm_file is non-NULL
 * then i_mmap_rwsem is taken here.
 */
int insert_vm_struct(struct mm_struct *mm, struct vm_area_struct *vma)
{
    struct vm_area_struct *prev;
    struct rb_node **rb_link, *rb_parent;

    if (find_vma_links(mm, vma->vm_start, vma->vm_end,
                       &prev, &rb_link, &rb_parent))
        return -ENOMEM;

    if (vma_is_anonymous(vma)) {
        vma->vm_pgoff = vma->vm_start >> PAGE_SHIFT;
    }

    vma_link(mm, vma, prev, rb_link, rb_parent);
    return 0;
}
EXPORT_SYMBOL(insert_vm_struct);

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vm_area_struct *
find_vma(struct mm_struct *mm, unsigned long addr)
{
    struct rb_node *rb_node;
    struct vm_area_struct *vma;

    rb_node = mm->mm_rb.rb_node;
    while (rb_node) {
        struct vm_area_struct *tmp;

        tmp = rb_entry(rb_node, struct vm_area_struct, vm_rb);

        if (tmp->vm_end > addr) {
            vma = tmp;
            if (tmp->vm_start <= addr)
                break;
            rb_node = rb_node->rb_left;
        } else
            rb_node = rb_node->rb_right;
    }

    return vma;
}
EXPORT_SYMBOL(find_vma);

struct vm_area_struct *
find_extend_vma(struct mm_struct *mm, unsigned long addr)
{
    unsigned long start;
    struct vm_area_struct *vma;

    addr &= PAGE_MASK;
    vma = find_vma(mm, addr);
    if (!vma)
        return NULL;
    if (vma->vm_start <= addr)
        return vma;

    panic("%s: (%lx <= %lx)!",
          __func__, vma->vm_start, addr);
}
EXPORT_SYMBOL(find_extend_vma);

pgprot_t protection_map[16] = {
    __P000, __P001, __P010, __P011, __P100, __P101, __P110, __P111,
    __S000, __S001, __S010, __S011, __S100, __S101, __S110, __S111
};

static inline pgprot_t arch_filter_pgprot(pgprot_t prot)
{
    return prot;
}

pgprot_t vm_get_page_prot(unsigned long vm_flags)
{
    pgprot_t ret =
        __pgprot(pgprot_val(protection_map[vm_flags &
                            (VM_READ|VM_WRITE|VM_EXEC|VM_SHARED)]) |
                 pgprot_val(arch_vm_get_page_prot(vm_flags)));

    return arch_filter_pgprot(ret);
}
EXPORT_SYMBOL(vm_get_page_prot);

int expand_downwards(struct vm_area_struct *vma,
                     unsigned long address)
{
    /* We must make sure the anon_vma is allocated. */
    /*
    if (unlikely(anon_vma_prepare(vma)))
        panic("out of memory!");
        */

    if (address < vma->vm_start) {
        unsigned long grow;

        grow = (vma->vm_start - address) >> PAGE_SHIFT;
        if (grow <= vma->vm_pgoff) {
            vma->vm_start = address;
            vma->vm_pgoff -= grow;
            vma_gap_update(vma);
        }
    }

    return 0;
}

int expand_stack(struct vm_area_struct *vma, unsigned long address)
{
    return expand_downwards(vma, address);
}
EXPORT_SYMBOL(expand_stack);

unsigned long
get_unmapped_area(struct file *file, unsigned long addr, unsigned long len,
                  unsigned long pgoff, unsigned long flags)
{
    unsigned long (*get_area)(struct file *, unsigned long,
                              unsigned long, unsigned long, unsigned long);

    /* Careful about overflows.. */
    if (len > TASK_SIZE)
        return -ENOMEM;

    get_area = current->mm->get_unmapped_area;

    addr = get_area(file, addr, len, pgoff, flags);
    if (IS_ERR_VALUE(addr))
        panic("bad addr!");

    printk("%s: addr(%lx) len(%lx)\n", __func__, addr, len);
    if (addr > TASK_SIZE - len)
        panic("out of memory!");
    if (offset_in_page(addr))
        panic("bad arg addr(%lx)!", addr);

    return addr;
}

static inline u64
file_mmap_size_max(struct file *file, struct inode *inode)
{
    if (S_ISREG(inode->i_mode))
        return MAX_LFS_FILESIZE;

    if (S_ISBLK(inode->i_mode))
        return MAX_LFS_FILESIZE;

    if (S_ISSOCK(inode->i_mode))
        return MAX_LFS_FILESIZE;

    /* Special "we do even unsigned file positions" case */
    if (file->f_mode & FMODE_UNSIGNED_OFFSET)
        return 0;

    /* Yes, random drivers might want more. But I'm tired of buggy drivers */
    return ULONG_MAX;
}

static inline bool
file_mmap_ok(struct file *file, struct inode *inode,
             unsigned long pgoff, unsigned long len)
{
    u64 maxsize = file_mmap_size_max(file, inode);

    if (maxsize && len > maxsize)
        return false;
    maxsize -= len;
    if (pgoff > maxsize >> PAGE_SHIFT)
        return false;
    return true;
}

/*
 * We account for memory if it's a private writeable mapping,
 * not hugepages and VM_NORESERVE wasn't set.
 */
static inline int
accountable_mapping(struct file *file, vm_flags_t vm_flags)
{
    return (vm_flags & (VM_NORESERVE | VM_SHARED | VM_WRITE)) == VM_WRITE;
}

/* Update vma->vm_page_prot to reflect vma->vm_flags. */
void vma_set_page_prot(struct vm_area_struct *vma)
{
    /* Todo: */
}

unsigned long
mmap_region(struct file *file, unsigned long addr,
            unsigned long len, vm_flags_t vm_flags, unsigned long pgoff,
            struct list_head *uf)
{
    int error;
    struct rb_node **rb_link, *rb_parent;
    struct vm_area_struct *vma, *prev, *merge;
    unsigned long charged = 0;
    struct mm_struct *mm = current->mm;

    /* Clear old maps */
    while (find_vma_links(mm, addr, addr + len, &prev, &rb_link, &rb_parent)) {
        if (do_munmap(mm, addr, len, uf))
            return -ENOMEM;
    }

    /*
     * Private writable mapping: check memory availability
     */
    if (accountable_mapping(file, vm_flags)) {
        charged = len >> PAGE_SHIFT;
        vm_flags |= VM_ACCOUNT;
    }

    /*
     * Determine the object being mapped and call the appropriate
     * specific mapper. the address has already been validated, but
     * not unmapped, but the maps are removed from the list.
     */
    vma = vm_area_alloc(mm);
    if (!vma)
        panic("out of memory!");

    vma->vm_start = addr;
    vma->vm_end = addr + len;
    vma->vm_flags = vm_flags;
    vma->vm_page_prot = vm_get_page_prot(vm_flags);
    vma->vm_pgoff = pgoff;

    if (file) {
        if (vm_flags & VM_SHARED)
            panic("VM_SHARED!");

        vma->vm_file = file;
        error = call_mmap(file, vma);
        if (error)
            panic("unmap and free vma!");

        if (unlikely(vm_flags != vma->vm_flags && prev))
            panic("vm_flags!");

        BUG_ON(addr != vma->vm_start);

        addr = vma->vm_start;
        vm_flags = vma->vm_flags;
    } else if (vm_flags & VM_SHARED) {
        panic("vm shared!");
    } else {
        panic("bad arg!");
    }

    vma_link(mm, vma, prev, rb_link, rb_parent);
    file = vma->vm_file;

    vma_set_page_prot(vma);
    return addr;
}

unsigned long
do_mmap(struct file *file, unsigned long addr,
        unsigned long len, unsigned long prot,
        unsigned long flags, unsigned long pgoff,
        unsigned long *populate, struct list_head *uf)
{
    vm_flags_t vm_flags;
    struct mm_struct *mm = current->mm;

    printk("%s: addr(%lx)\n", __func__, addr);

    *populate = 0;

    if (!len)
        return -EINVAL;

    /* Careful about overflows.. */
    len = PAGE_ALIGN(len);
    if (!len)
        return -ENOMEM;

    /* offset overflow? */
    if ((pgoff + (len >> PAGE_SHIFT)) < pgoff)
        return -EOVERFLOW;

    /* Obtain the address to map to. we verify (or select) it and ensure
     * that it represents a valid section of the address space.
     */
    addr = get_unmapped_area(file, addr, len, pgoff, flags);
    if (IS_ERR_VALUE(addr))
        return addr;

    /* Do simple checking here so the lower-level routines won't have
     * to. we assume access permissions have been handled by the open
     * of the memory object, so we don't do any here.
     */
    vm_flags = calc_vm_prot_bits(prot) | calc_vm_flag_bits(flags) |
        mm->def_flags | VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC;

    if (file) {
        unsigned long flags_mask;
        struct inode *inode = file_inode(file);

        if (!file_mmap_ok(file, inode, pgoff, len))
            panic("overflow!");

        //flags_mask = LEGACY_MAP_MASK | file->f_op->mmap_supported_flags;
        flags_mask = LEGACY_MAP_MASK;

        switch (flags & MAP_TYPE) {
            case MAP_SHARED:
                panic("MAP_SHARED!");
            case MAP_SHARED_VALIDATE:
                panic("MAP_SHARED_VALIDATE!");
            case MAP_PRIVATE:
                if (!(file->f_mode & FMODE_READ))
                    return -EACCES;

                if (!file->f_op->mmap)
                    return -ENODEV;
                if (vm_flags & (VM_GROWSDOWN|VM_GROWSUP))
                    return -EINVAL;

                break;
            default:
                panic("bad flags!");
        }
    } else {
        panic("no file!");
    }

    if (flags & MAP_NORESERVE)
        panic("MAP_NORESERVE!");

    addr = mmap_region(file, addr, len, vm_flags, pgoff, uf);
    if (!IS_ERR_VALUE(addr) &&
        ((vm_flags & VM_LOCKED) ||
         (flags & (MAP_POPULATE | MAP_NONBLOCK)) == MAP_POPULATE))
        panic("set poplate!");

    return addr;
}
EXPORT_SYMBOL(do_mmap);

int do_brk_flags(unsigned long addr, unsigned long len,
                 unsigned long flags, struct list_head *uf)
{
    int error;
    unsigned long mapped_addr;
    struct vm_area_struct *vma, *prev;
    struct rb_node **rb_link, *rb_parent;
    pgoff_t pgoff = addr >> PAGE_SHIFT;
    struct mm_struct *mm = current->mm;

    printk("### %s: 1 (%lx, %lx)\n", __func__, addr, len);
    /* Until we need other flags, refuse anything except VM_EXEC. */
    if ((flags & (~VM_EXEC)) != 0)
        return -EINVAL;
    flags |= VM_DATA_DEFAULT_FLAGS | VM_ACCOUNT | mm->def_flags;

    mapped_addr = get_unmapped_area(NULL, addr, len, 0, MAP_FIXED);
    if (IS_ERR_VALUE(mapped_addr))
        return mapped_addr;

    /*
     * Clear old maps.  this also does some error checking for us
     */
    while (find_vma_links(mm, addr, addr + len,
                          &prev, &rb_link, &rb_parent))
        panic("find vma links error!");

    /*
     * create a vma struct for an anonymous mapping
     */
    vma = vm_area_alloc(mm);
    if (!vma)
        panic("out of memory!");

    vma_set_anonymous(vma);
    vma->vm_start = addr;
    vma->vm_end = addr + len;
    vma->vm_pgoff = pgoff;
    vma->vm_flags = flags;
    vma->vm_page_prot = vm_get_page_prot(flags);
    vma_link(mm, vma, prev, rb_link, rb_parent);
    mm->total_vm += len >> PAGE_SHIFT;
    mm->data_vm += len >> PAGE_SHIFT;
    return 0;
}
EXPORT_SYMBOL(do_brk_flags);

int vm_brk_flags(unsigned long addr, unsigned long request,
                 unsigned long flags)
{
    int ret;
    bool populate;
    unsigned long len;
    LIST_HEAD(uf);
    struct mm_struct *mm = current->mm;

    len = PAGE_ALIGN(request);
    if (len < request)
        return -ENOMEM;
    if (!len)
        return 0;

    ret = do_brk_flags(addr, len, flags, &uf);
    populate = ((mm->def_flags & VM_LOCKED) != 0);
    if (populate && !ret)
        panic("can not populate!");
    return ret;
}
EXPORT_SYMBOL(vm_brk_flags);

int vm_munmap(unsigned long start, size_t len)
{
    printk("%s: TODO: NOT IMPLEMENTED yet!\n", __func__);
    return 0;
    //return __vm_munmap(start, len, false);
}
EXPORT_SYMBOL(vm_munmap);

static void __vma_link_file(struct vm_area_struct *vma)
{
    struct file *file;

    file = vma->vm_file;
    if (file) {
        panic("%s: todo:", __func__);
#if 0
        struct address_space *mapping = file->f_mapping;

        if (vma->vm_flags & VM_DENYWRITE)
            atomic_dec(&file_inode(file)->i_writecount);
        if (vma->vm_flags & VM_SHARED)
            atomic_inc(&mapping->i_mmap_writable);

        flush_dcache_mmap_lock(mapping);
        vma_interval_tree_insert(vma, &mapping->i_mmap);
        flush_dcache_mmap_unlock(mapping);
#endif
    }
}

/*
 * We cannot adjust vm_start, vm_end, vm_pgoff fields of a vma that
 * is already present in an i_mmap tree without adjusting the tree.
 * The following helper function should be used when such adjustments
 * are necessary.  The "insert" vma (if any) is to be inserted
 * before we drop the necessary locks.
 */
int __vma_adjust(struct vm_area_struct *vma,
                 unsigned long start, unsigned long end,
                 pgoff_t pgoff, struct vm_area_struct *insert,
                 struct vm_area_struct *expand)
{
    struct anon_vma *anon_vma = NULL;
    struct address_space *mapping = NULL;
    struct rb_root_cached *root = NULL;
    struct file *file = vma->vm_file;

    struct vm_area_struct *next = vma->vm_next, *orig_vma = vma;

    if (next && !insert) {
        panic("%s: step1!", __func__);
    }

    if (file) {
        mapping = file->f_mapping;
        root = &mapping->i_mmap;

        if (insert) {
            /*
             * Put into interval tree now, so instantiated pages
             * are visible to arm/parisc __flush_dcache_page
             * throughout; but we cannot insert into address
             * space until vma start or end is updated.
             */
            __vma_link_file(insert);
        }
    }

    anon_vma = vma->anon_vma;
    panic("%s: todo!", __func__);
}

/*
 * __split_vma() bypasses sysctl_max_map_count checking.  We use this where it
 * has already been checked or doesn't make sense to fail.
 */
int __split_vma(struct mm_struct *mm, struct vm_area_struct *vma,
                unsigned long addr, int new_below)
{
    int err;
    struct vm_area_struct *new;

    new = vm_area_dup(vma);
    if (!new)
        return -ENOMEM;

    if (new_below)
        new->vm_end = addr;
    else {
        new->vm_start = addr;
        new->vm_pgoff += ((addr - vma->vm_start) >> PAGE_SHIFT);
    }

    if (new_below)
        err = vma_adjust(vma, addr, vma->vm_end, vma->vm_pgoff +
                         ((addr - new->vm_start) >> PAGE_SHIFT), new);
    else
        err = vma_adjust(vma, vma->vm_start, addr, vma->vm_pgoff, new);

    /* Success. */
    if (!err)
        return 0;

    panic("%s: todo!", __func__);
}

/* Munmap is split into 2 main parts -- this part which finds
 * what needs doing, and the areas themselves, which do the
 * work.  This now handles partial unmappings.
 * Jeremy Fitzhardinge <jeremy@goop.org>
 */
int __do_munmap(struct mm_struct *mm, unsigned long start, size_t len,
                struct list_head *uf, bool downgrade)
{
    unsigned long end;
    struct vm_area_struct *vma, *prev, *last;

    if ((offset_in_page(start)) ||
        start > TASK_SIZE || len > TASK_SIZE-start)
        return -EINVAL;

    len = PAGE_ALIGN(len);
    end = start + len;
    if (len == 0)
        return -EINVAL;

    /* Find the first overlapping VMA */
    vma = find_vma(mm, start);
    if (!vma)
        return 0;
    prev = vma->vm_prev;
    /* we have  start < vma->vm_end  */

    /* if it doesn't overlap, we have nothing.. */
    if (vma->vm_start >= end)
        return 0;

    /*
     * If we need to split any vma, do it now to save pain later.
     *
     * Note: mremap's move_vma VM_ACCOUNT handling assumes a partially
     * unmapped vm_area_struct will remain in use: so lower split_vma
     * places tmp vma above, and higher split_vma places tmp vma below.
     */
    if (start > vma->vm_start) {
        int error;

        /*
         * Make sure that map_count on return from munmap() will
         * not exceed its limit; but let map_count go just above
         * its limit temporarily, to help free resources as expected.
         */
        if (end < vma->vm_end)
            return -ENOMEM;

        error = __split_vma(mm, vma, start, 0);
        if (error)
            return error;
        prev = vma;
    }

    panic("%s: todo!", __func__);
}

int do_munmap(struct mm_struct *mm, unsigned long start, size_t len,
              struct list_head *uf)
{
    return __do_munmap(mm, start, len, uf, false);
}

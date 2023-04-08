// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <fork.h>
#include <mman.h>
#include <stat.h>
#include <errno.h>
#include <export.h>
#include <limits.h>
#include <rbtree_augmented.h>
#include <current.h>
#include <processor.h>

#ifndef arch_get_mmap_end
#define arch_get_mmap_end(addr) (TASK_SIZE)
#endif

#ifndef arch_get_mmap_base
#define arch_get_mmap_base(addr, base) (base)
#endif

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

/*
 * Same as find_vma, but also return a pointer to the previous VMA in *pprev.
 */
struct vm_area_struct *
find_vma_prev(struct mm_struct *mm, unsigned long addr,
              struct vm_area_struct **pprev)
{
    struct vm_area_struct *vma;

    vma = find_vma(mm, addr);
    if (vma) {
        *pprev = vma->vm_prev;
    } else {
        struct rb_node *rb_node = rb_last(&mm->mm_rb);

        *pprev = rb_node ? rb_entry(rb_node, struct vm_area_struct, vm_rb) : NULL;
    }
    return vma;
}
EXPORT_SYMBOL(find_vma_prev);

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
        vma_set_anonymous(vma);
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

    pr_debug("%s: addr(%lx)\n", __func__, addr);

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

        if (inode && !file_mmap_ok(file, inode, pgoff, len))
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
        switch (flags & MAP_TYPE) {
        case MAP_SHARED:
            if (vm_flags & (VM_GROWSDOWN|VM_GROWSUP))
                return -EINVAL;
            /*
             * Ignore pgoff.
             */
            pgoff = 0;
            vm_flags |= VM_SHARED | VM_MAYSHARE;
            break;
        case MAP_PRIVATE:
            /*
             * Set pgoff according to addr for anon_vma.
             */
            pgoff = addr >> PAGE_SHIFT;
            break;
        default:
            return -EINVAL;
        }
    }

    if (flags & MAP_NORESERVE)
        panic("MAP_NORESERVE!");

    pr_info("%s: ...\n", __func__);
    addr = mmap_region(file, addr, len, vm_flags, pgoff, uf);
    if (!IS_ERR_VALUE(addr) &&
        ((vm_flags & VM_LOCKED) ||
         (flags & (MAP_POPULATE | MAP_NONBLOCK)) == MAP_POPULATE))
        panic("set poplate!");

    pr_info("%s: addr(%lx) end!\n", __func__, addr);
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

static int __vm_munmap(unsigned long start, size_t len, bool downgrade)
{
    int ret;
    struct mm_struct *mm = current->mm;
    LIST_HEAD(uf);

    /*
    if (mmap_write_lock_killable(mm))
        return -EINTR;
        */

    ret = __do_munmap(mm, start, len, &uf, downgrade);
    /*
     * Returning 1 indicates mmap_lock is downgraded.
     * But 1 is not legal return value of vm_munmap() and munmap(), reset
     * it to 0 before return.
     */
    if (ret == 1) {
        //mmap_read_unlock(mm);
        ret = 0;
    } else {
        //mmap_write_unlock(mm);
    }

    return ret;
}

int vm_munmap(unsigned long start, size_t len)
{
    return __vm_munmap(start, len, false);
}
EXPORT_SYMBOL(vm_munmap);

static void __vma_link_file(struct vm_area_struct *vma)
{
    struct file *file;

    file = vma->vm_file;
    if (file) {
        struct address_space *mapping = file->f_mapping;

        if (vma->vm_flags & VM_DENYWRITE)
            atomic_dec(&file_inode(file)->i_writecount);
        if (vma->vm_flags & VM_SHARED)
            atomic_inc(&mapping->i_mmap_writable);

        vma_interval_tree_insert(vma, &mapping->i_mmap);
    }
}

/*
 * Helper for vma_adjust() in the split_vma insert case: insert a vma into the
 * mm's list and rbtree.  It has already been inserted into the interval tree.
 */
static void __insert_vm_struct(struct mm_struct *mm, struct vm_area_struct *vma)
{
    struct vm_area_struct *prev;
    struct rb_node **rb_link, *rb_parent;

    if (find_vma_links(mm, vma->vm_start, vma->vm_end,
               &prev, &rb_link, &rb_parent))
        BUG();
    __vma_link(mm, vma, prev, rb_link, rb_parent);
    //mm->map_count++;
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
    struct mm_struct *mm = vma->vm_mm;
    struct anon_vma *anon_vma = NULL;
    struct address_space *mapping = NULL;
    struct rb_root_cached *root = NULL;
    struct file *file = vma->vm_file;
    bool start_changed = false, end_changed = false;
    long adjust_next = 0;
    int remove_next = 0;

    struct vm_area_struct *next = vma->vm_next, *orig_vma = vma;

    if (next && !insert) {
        panic("%s: step1!", __func__);
    }

    if (file && file->f_mapping) {
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
    if (!anon_vma && adjust_next)
        anon_vma = next->anon_vma;
    if (anon_vma) {
        /*
        BUG_ON(adjust_next && next->anon_vma && anon_vma != next->anon_vma);
        anon_vma_lock_write(anon_vma);
        anon_vma_interval_tree_pre_update_vma(vma);
        if (adjust_next)
            anon_vma_interval_tree_pre_update_vma(next);
        */
        panic("%s: anon_vma!\n", __func__);
    }
    pr_debug("%s: anon_vma (%p)\n", __func__, anon_vma);

    if (root) {
        vma_interval_tree_remove(vma, root);
        if (adjust_next)
            vma_interval_tree_remove(next, root);
    }

    if (start != vma->vm_start) {
        vma->vm_start = start;
        start_changed = true;
    }
    if (end != vma->vm_end) {
        vma->vm_end = end;
        end_changed = true;
    }
    vma->vm_pgoff = pgoff;
    if (adjust_next) {
        next->vm_start += adjust_next << PAGE_SHIFT;
        next->vm_pgoff += adjust_next;
    }

    if (root) {
        if (adjust_next)
            vma_interval_tree_insert(next, root);
        vma_interval_tree_insert(vma, root);
    }

    if (remove_next) {
#if 0
        /*
         * vma_merge has merged next into vma, and needs
         * us to remove next before dropping the locks.
         */
        if (remove_next != 3)
            __vma_unlink_common(mm, next, next);
        else
            /*
             * vma is not before next if they've been
             * swapped.
             *
             * pre-swap() next->vm_start was reduced so
             * tell validate_mm_rb to ignore pre-swap()
             * "next" (which is stored in post-swap()
             * "vma").
             */
            __vma_unlink_common(mm, next, vma);
        if (file)
            __remove_shared_vm_struct(next, file, mapping);
#endif
        panic("remove_next!\n");
    } else if (insert) {
        /*
         * split_vma has split insert from vma, and needs
         * us to insert it before dropping the locks
         * (it may either follow vma or precede it).
         */
        __insert_vm_struct(mm, insert);
    } else {
        if (start_changed)
            vma_gap_update(vma);
        if (end_changed) {
            if (!next)
                mm->highest_vm_end = vm_end_gap(vma);
            else if (!adjust_next)
                vma_gap_update(next);
        }
    }

    if (anon_vma) {
        /*
        anon_vma_interval_tree_post_update_vma(vma);
        if (adjust_next)
            anon_vma_interval_tree_post_update_vma(next);
        anon_vma_unlock_write(anon_vma);
        */
        panic("anon_vma!\n");
    }
#if 0
    if (mapping)
        i_mmap_unlock_write(mapping);
#endif

    if (remove_next) {
        panic("remove_next!\n");
    }
    return 0;
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

static void
__vma_rb_erase(struct vm_area_struct *vma, struct rb_root *root)
{
    /*
     * Note rb_erase_augmented is a fairly large inline function,
     * so make sure we instantiate it only once with our desired
     * augmented rbtree callbacks.
     */
    rb_erase_augmented(&vma->vm_rb, root, &vma_gap_callbacks);
}

static __always_inline void
vma_rb_erase(struct vm_area_struct *vma, struct rb_root *root)
{
    __vma_rb_erase(vma, root);
}

/*
 * Create a list of vma's touched by the unmap, removing them from the mm's
 * vma list as we go..
 */
static bool
detach_vmas_to_be_unmapped(struct mm_struct *mm,
                           struct vm_area_struct *vma,
                           struct vm_area_struct *prev,
                           unsigned long end)
{
    struct vm_area_struct **insertion_point;
    struct vm_area_struct *tail_vma = NULL;

    insertion_point = (prev ? &prev->vm_next : &mm->mmap);
    vma->vm_prev = NULL;
    do {
        vma_rb_erase(vma, &mm->mm_rb);
        //mm->map_count--;
        tail_vma = vma;
        vma = vma->vm_next;
    } while (vma && vma->vm_start < end);
    *insertion_point = vma;
    if (vma) {
        vma->vm_prev = prev;
        vma_gap_update(vma);
    } else
        mm->highest_vm_end = prev ? vm_end_gap(prev) : 0;
    tail_vma->vm_next = NULL;

    /* Kill the cache */
    //vmacache_invalidate(mm);

    /*
     * Do not downgrade mmap_lock if we are next to VM_GROWSDOWN or
     * VM_GROWSUP VMA. Such VMAs can change their size under
     * down_read(mmap_lock) and collide with the VMA we are about to unmap.
     */
    if (vma && (vma->vm_flags & VM_GROWSDOWN))
        return false;
    if (prev && (prev->vm_flags & VM_GROWSUP))
        return false;
    return true;

    panic("%s: todo!\n", __func__);
}

/*
 * Get rid of page table information in the indicated region.
 *
 * Called with the mm semaphore held.
 */
static void
unmap_region(struct mm_struct *mm,
             struct vm_area_struct *vma,
             struct vm_area_struct *prev,
             unsigned long start,
             unsigned long end)
{
#if 0
    struct vm_area_struct *next = prev ? prev->vm_next : mm->mmap;
    struct mmu_gather tlb;

    lru_add_drain();
    tlb_gather_mmu(&tlb, mm, start, end);
    update_hiwater_rss(mm);
    unmap_vmas(&tlb, vma, start, end);
    free_pgtables(&tlb, vma, prev ? prev->vm_end : FIRST_USER_ADDRESS,
                 next ? next->vm_start : USER_PGTABLES_CEILING);
    tlb_finish_mmu(&tlb, start, end);
#endif
    pr_warn("%s: NO Implementation!\n", __func__);
}

/*
 * Close a vm structure and free it, returning the next.
 */
static struct vm_area_struct *remove_vma(struct vm_area_struct *vma)
{
    struct vm_area_struct *next = vma->vm_next;

    //might_sleep();
#if 0
    if (vma->vm_ops && vma->vm_ops->close)
        vma->vm_ops->close(vma);
#endif
#if 0
    if (vma->vm_file)
        fput(vma->vm_file);
#endif
    //mpol_put(vma_policy(vma));
    vm_area_free(vma);
    return next;
}

/*
 * Ok - we have the memory areas we should free on the vma list,
 * so release them, and do the vma updates.
 *
 * Called with the mm semaphore held.
 */
static void remove_vma_list(struct mm_struct *mm,
                            struct vm_area_struct *vma)
{
    unsigned long nr_accounted = 0;

    /* Update high watermark before we lower total_vm */
    //update_hiwater_vm(mm);
    do {
        long nrpages = vma_pages(vma);

        if (vma->vm_flags & VM_ACCOUNT)
            nr_accounted += nrpages;
        //vm_stat_account(mm, vma->vm_flags, -nrpages);
        vma = remove_vma(vma);
    } while (vma);
    //vm_unacct_memory(nr_accounted);
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

    /* Does it split the last one? */
    last = find_vma(mm, end);
    if (last && end > last->vm_start) {
        int error = __split_vma(mm, last, end, 1);
        if (error)
            return error;
    }
    vma = prev ? prev->vm_next : mm->mmap;

    /*
     * unlock any mlock()ed ranges before detaching vmas
     */
#if 0
    if (mm->locked_vm) {
        struct vm_area_struct *tmp = vma;
        while (tmp && tmp->vm_start < end) {
            if (tmp->vm_flags & VM_LOCKED) {
                mm->locked_vm -= vma_pages(tmp);
                munlock_vma_pages_all(tmp);
            }

            tmp = tmp->vm_next;
        }
    }
#endif

    /* Detach vmas from rbtree */
    if (!detach_vmas_to_be_unmapped(mm, vma, prev, end))
        downgrade = false;

#if 0
    if (downgrade)
        mmap_write_downgrade(mm);
#endif

    unmap_region(mm, vma, prev, start, end);

    /* Fix up all other VM information */
    remove_vma_list(mm, vma);

    return downgrade ? 1 : 0;
}

int do_munmap(struct mm_struct *mm, unsigned long start, size_t len,
              struct list_head *uf)
{
    return __do_munmap(mm, start, len, uf, false);
}

static unsigned long
unmapped_area_topdown(struct vm_unmapped_area_info *info)
{
    struct mm_struct *mm = current->mm;
    struct vm_area_struct *vma;
    unsigned long length, low_limit, high_limit, gap_start, gap_end;

    /* Adjust search length to account for worst case alignment overhead */
    length = info->length + info->align_mask;
    if (length < info->length)
        return -ENOMEM;

    /*
     * Adjust search limits by the desired length.
     * See implementation comment at top of unmapped_area().
     */
    gap_end = info->high_limit;
    if (gap_end < length)
        return -ENOMEM;
    high_limit = gap_end - length;

    if (info->low_limit > high_limit)
        return -ENOMEM;
    low_limit = info->low_limit + length;

    /* Check highest gap, which does not precede any rbtree node */
    gap_start = mm->highest_vm_end;
    if (gap_start <= high_limit)
        goto found_highest;

    /* Check if rbtree root looks promising */
    if (RB_EMPTY_ROOT(&mm->mm_rb))
        return -ENOMEM;
    vma = rb_entry(mm->mm_rb.rb_node, struct vm_area_struct, vm_rb);
    if (vma->rb_subtree_gap < length)
        return -ENOMEM;

    while (true) {
        /* Visit right subtree if it looks promising */
        gap_start = vma->vm_prev ? vm_end_gap(vma->vm_prev) : 0;
        if (gap_start <= high_limit && vma->vm_rb.rb_right) {
            struct vm_area_struct *right =
                rb_entry(vma->vm_rb.rb_right,
                     struct vm_area_struct, vm_rb);
            if (right->rb_subtree_gap >= length) {
                vma = right;
                continue;
            }
        }

check_current:
        /* Check if current node has a suitable gap */
        gap_end = vm_start_gap(vma);
        if (gap_end < low_limit)
            return -ENOMEM;
        if (gap_start <= high_limit &&
            gap_end > gap_start && gap_end - gap_start >= length)
            goto found;

        /* Visit left subtree if it looks promising */
        if (vma->vm_rb.rb_left) {
            struct vm_area_struct *left =
                rb_entry(vma->vm_rb.rb_left,
                     struct vm_area_struct, vm_rb);
            if (left->rb_subtree_gap >= length) {
                vma = left;
                continue;
            }
        }

        /* Go back up the rbtree to find next candidate node */
        while (true) {
            struct rb_node *prev = &vma->vm_rb;
            if (!rb_parent(prev))
                return -ENOMEM;
            vma = rb_entry(rb_parent(prev),
                       struct vm_area_struct, vm_rb);
            if (prev == vma->vm_rb.rb_right) {
                gap_start = vma->vm_prev ?
                    vm_end_gap(vma->vm_prev) : 0;
                goto check_current;
            }
        }
    }

    panic("%s: todo!\n", __func__);
found:
    /* We found a suitable gap. Clip it with the original high_limit. */
    if (gap_end > info->high_limit)
        gap_end = info->high_limit;

found_highest:
    /* Compute highest gap address at the desired alignment */
    gap_end -= info->length;
    gap_end -= (gap_end - info->align_offset) & info->align_mask;

    BUG_ON(gap_end < info->low_limit);
    BUG_ON(gap_end < gap_start);
    return gap_end;
}

static unsigned long unmapped_area(struct vm_unmapped_area_info *info)
{
    panic("%s: todo!\n", __func__);
}

/*
 * Search for an unmapped address range.
 *
 * We are looking for a range that:
 * - does not intersect with any VMA;
 * - is contained within the [low_limit, high_limit) interval;
 * - is at least the desired size.
 * - satisfies (begin_addr & align_mask) == (align_offset & align_mask)
 */
unsigned long vm_unmapped_area(struct vm_unmapped_area_info *info)
{
    unsigned long addr;

    if (info->flags & VM_UNMAPPED_AREA_TOPDOWN)
        addr = unmapped_area_topdown(info);
    else
        addr = unmapped_area(info);

    return addr;
}

unsigned long
arch_get_unmapped_area_topdown(struct file *filp, unsigned long addr,
                               unsigned long len, unsigned long pgoff,
                               unsigned long flags)
{
    struct vm_area_struct *vma, *prev;
    struct vm_unmapped_area_info info;
    struct mm_struct *mm = current->mm;
    const unsigned long mmap_end = arch_get_mmap_end(addr);

    if (flags & MAP_FIXED)
        return addr;

    /* requesting a specific address */
    if (addr) {
        addr = PAGE_ALIGN(addr);
        vma = find_vma_prev(mm, addr, &prev);
        if (mmap_end - len >= addr &&
            (!vma || addr + len <= vm_start_gap(vma)) &&
            (!prev || addr >= vm_end_gap(prev)))
            return addr;
    }

    info.flags = VM_UNMAPPED_AREA_TOPDOWN;
    info.length = len;
    //info.low_limit = max(PAGE_SIZE, mmap_min_addr);
    info.low_limit = PAGE_SIZE;
    info.high_limit = arch_get_mmap_base(addr, mm->mmap_base);
    info.align_mask = 0;
    info.align_offset = 0;
    addr = vm_unmapped_area(&info);

    /*
     * A failed mmap() very likely causes application failure,
     * so fall back to the bottom-up function here. This scenario
     * can happen with large stack limits and large mmap()
     * allocations.
     */
    if (offset_in_page(addr)) {
        BUG_ON(addr != -ENOMEM);
        info.flags = 0;
        info.low_limit = TASK_UNMAPPED_BASE;
        info.high_limit = mmap_end;
        addr = vm_unmapped_area(&info);
    }

    return addr;
}

unsigned long
ksys_mmap_pgoff(unsigned long addr, unsigned long len,
                unsigned long prot, unsigned long flags,
                unsigned long fd, unsigned long pgoff)
{
    struct file *file = NULL;
    unsigned long retval;

    if (!(flags & MAP_ANONYMOUS)) {
        panic("%s: MAP_ANONYMOUS\n", __func__);
    } else if (flags & MAP_HUGETLB) {
        panic("%s: MAP_HUGETLB\n", __func__);
    }

    flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);

    retval = vm_mmap_pgoff(file, addr, len, prot, flags, pgoff);

 out_fput:
#if 0
    if (file)
        fput(file);
#endif
    return retval;
}

long _riscv_sys_mmap(unsigned long addr, unsigned long len,
                     unsigned long prot, unsigned long flags,
                     unsigned long fd, off_t offset,
                     unsigned long page_shift_offset)
{
    printk("%s: step1\n", __func__);
    if (unlikely(offset & (~PAGE_MASK >> page_shift_offset)))
        return -EINVAL;

    if ((prot & PROT_WRITE) && (prot & PROT_EXEC))
        if (unlikely(!(prot & PROT_READ)))
            return -EINVAL;

    return ksys_mmap_pgoff(addr, len, prot, flags, fd,
                           offset >> (PAGE_SHIFT - page_shift_offset));
}

void
init_mmap(void)
{
    riscv_sys_mmap = _riscv_sys_mmap;
    do_vm_munmap = __vm_munmap;
}

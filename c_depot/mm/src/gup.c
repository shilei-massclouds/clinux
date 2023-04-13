// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <export.h>
#include <current.h>

__get_user_pages_t __get_user_pages_cb = NULL;
EXPORT_SYMBOL(__get_user_pages_cb);

struct follow_page_context {
    //struct dev_pagemap *pgmap;
    void *pgmap;
    unsigned int page_mask;
};

/**
 * populate_vma_page_range() -  populate a range of pages in the vma.
 * @vma:   target vma
 * @start: start address
 * @end:   end address
 * @locked: whether the mmap_lock is still held
 *
 * This takes care of mlocking the pages too if VM_LOCKED is set.
 *
 * Return either number of pages pinned in the vma, or a negative error
 * code on error.
 *
 * vma->vm_mm->mmap_lock must be held.
 *
 * If @locked is NULL, it may be held for read or write and will
 * be unperturbed.
 *
 * If @locked is non-NULL, it must held for read only and may be
 * released.  If it's released, *@locked will be set to 0.
 */
long populate_vma_page_range(struct vm_area_struct *vma,
                             unsigned long start, unsigned long end,
                             int *locked)
{
    struct mm_struct *mm = vma->vm_mm;
    unsigned long nr_pages = (end - start) / PAGE_SIZE;
    int gup_flags;

    BUG_ON(start & ~PAGE_MASK);
    BUG_ON(end   & ~PAGE_MASK);
    BUG_ON(start < vma->vm_start);
    BUG_ON(end   > vma->vm_end);
    //mmap_assert_locked(mm);

    gup_flags = FOLL_TOUCH | FOLL_POPULATE | FOLL_MLOCK;
    if (vma->vm_flags & VM_LOCKONFAULT)
        gup_flags &= ~FOLL_POPULATE;
    /*
     * We want to touch writable mappings with a write fault in order
     * to break COW, except for shared mappings because these don't COW
     * and we would not want to dirty them for nothing.
     */
    if ((vma->vm_flags & (VM_WRITE | VM_SHARED)) == VM_WRITE)
        gup_flags |= FOLL_WRITE;

    /*
     * We want mlock to succeed for regions that have any permissions
     * other than PROT_NONE.
     */
#if 0
    if (vma_is_accessible(vma))
        gup_flags |= FOLL_FORCE;
#endif

    if (__get_user_pages_cb == NULL) {
        panic("__get_user_pages_cb NOT registered!");
    }

    /*
     * We made sure addr is within a VMA, so the following will
     * not result in a stack expansion that recurses back here.
     */
    return __get_user_pages_cb(mm, start, nr_pages, gup_flags,
                               NULL, NULL, locked);
}

int
__mm_populate(unsigned long start, unsigned long len, int ignore_errors)
{
    struct mm_struct *mm = current->mm;
    unsigned long end, nstart, nend;
    struct vm_area_struct *vma = NULL;
    int locked = 0;
    long ret = 0;

    end = start + len;

    for (nstart = start; nstart < end; nstart = nend) {
        /*
         * We want to fault in pages for [nstart; end) address range.
         * Find first corresponding VMA.
         */
        if (!locked) {
            locked = 1;
            //mmap_read_lock(mm);
            vma = find_vma(mm, nstart);
        } else if (nstart >= vma->vm_end)
            vma = vma->vm_next;
        if (!vma || vma->vm_start >= end)
            break;
        /*
         * Set [nstart; nend) to intersection of desired address
         * range with the first VMA. Also, skip undesirable VMA types.
         */
        nend = min(end, vma->vm_end);
        if (vma->vm_flags & (VM_IO | VM_PFNMAP))
            continue;
        if (nstart < vma->vm_start)
            nstart = vma->vm_start;
        /*
         * Now fault in a range of pages. populate_vma_page_range()
         * double checks the vma flags, so that it won't mlock pages
         * if the vma was already munlocked.
         */
        ret = populate_vma_page_range(vma, nstart, nend, &locked);
        if (ret < 0) {
            if (ignore_errors) {
                ret = 0;
                continue;   /* continue at next VMA */
            }
            break;
        }
        nend = nstart + ret * PAGE_SIZE;
        ret = 0;
    }
#if 0
    if (locked)
        mmap_read_unlock(mm);
#endif
    return ret; /* 0 or negative error code */
}
EXPORT_SYMBOL(__mm_populate);

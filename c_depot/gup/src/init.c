// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <errno.h>
#include <types.h>
#include <export.h>
#include <pgtable.h>
#include <mm_types.h>
#include <current.h>

static struct page *
no_page_table(struct vm_area_struct *vma, unsigned int flags)
{
    return NULL;
}

static struct page *
follow_page_pte(struct vm_area_struct *vma,
                unsigned long address, pmd_t *pmd, unsigned int flags)
{
    struct page *page;
    pte_t *ptep, pte;

 retry:
    pr_debug("%s: 1 pmd(%lx) address(%lx) (%lx, %lx)\n",
           __func__, pmd->pmd, address, vma->vm_end, vma->vm_start);

    ptep = pte_offset_map_lock(mm, pmd, address);
    pte = *ptep;
    if (!pte_present(pte)) {
        /* For unikernel, populate it! */
        if (1) {
            pgd_t *pgd = current->mm->pgd;
            unsigned long start;
            for (start = vma->vm_start; start < vma->vm_end; start += PAGE_SIZE) {
                unsigned long pa = buddy_phys_alloc(PAGE_SIZE, PAGE_SIZE);
                create_pgd_mapping(pgd, start, pa, PAGE_SIZE, PAGE_KERNEL_EXEC);
            }
            goto retry;
        } else {
            panic("pte NOT present! (%lx)\n", pte.pte);
        }
    }

    page = vm_normal_page(vma, address, pte);
    if (!page)
        panic("bad page!");

    if (flags & FOLL_TOUCH) {
        if ((flags & FOLL_WRITE) && !pte_dirty(pte) && !PageDirty(page))
            set_page_dirty(page);

        /*
         * pte_mkyoung() would be more correct here, but atomic care
         * is needed to avoid losing the dirty bit: it is easier to use
         * mark_page_accessed().
         */
        //mark_page_accessed(page);
    }

    return page;
}

static struct page *
follow_pmd_mask(struct vm_area_struct *vma,
                unsigned long address, pgd_t *pgdp,
                unsigned int flags)
{
    pmd_t *pmd, pmdval;

    pmd = pmd_offset(pgdp, address);
    /*
     * The READ_ONCE() will stabilize the pmdval in a register or
     * on the stack so that it will stop changing under the code.
     */
    pmdval = READ_ONCE(*pmd);
    if (pmd_none(pmdval))
        return no_page_table(vma, flags);

    if (!pmd_present(pmdval))
        panic("pmd not present!");

    return follow_page_pte(vma, address, pmd, flags);
}

static struct page *
follow_page_mask(struct vm_area_struct *vma,
                 unsigned long address, unsigned int flags)
{
    pgd_t *pgd;
    struct mm_struct *mm = vma->vm_mm;

    pgd = pgd_offset(mm, address);

    if (pgd_none(*pgd))
        return no_page_table(vma, flags);

    return follow_pmd_mask(vma, address, pgd, flags);
}

static int faultin_page(struct vm_area_struct *vma,
                        unsigned long address,
                        unsigned int *flags, int *locked)
{
    vm_fault_t ret;
    unsigned int fault_flags = 0;

    /* mlock all present pages, but do not fault in new pages */
    if ((*flags & (FOLL_POPULATE | FOLL_MLOCK)) == FOLL_MLOCK)
        return -ENOENT;
    if (*flags & FOLL_WRITE)
        fault_flags |= FAULT_FLAG_WRITE;
    if (*flags & FOLL_REMOTE)
        fault_flags |= FAULT_FLAG_REMOTE;
    if (locked)
        fault_flags |= FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
    if (*flags & FOLL_NOWAIT)
        fault_flags |= FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_RETRY_NOWAIT;
    if (*flags & FOLL_TRIED) {
        /*
         * Note: FAULT_FLAG_ALLOW_RETRY and FAULT_FLAG_TRIED
         * can co-exist
         */
        fault_flags |= FAULT_FLAG_TRIED;
    }

    pr_debug("%s: address(%lx) fault_flags(%x)\n", __func__, address, fault_flags);
    ret = handle_mm_fault(vma, address, fault_flags, NULL);
    if (ret & VM_FAULT_ERROR)
        panic("handle mm fault error!");

    return 0;
}

/**
 * __get_user_pages() - pin user pages in memory
 * @mm:     mm_struct of target mm
 * @start:  starting user address
 * @nr_pages:   number of pages from start to pin
 * @gup_flags:  flags modifying pin behaviour
 * @pages:  array that receives pointers to the pages pinned.
 *      Should be at least nr_pages long. Or NULL, if caller
 *      only intends to ensure the pages are faulted in.
 * @vmas:   array of pointers to vmas corresponding to each page.
 *      Or NULL if the caller does not require them.
 * @locked:     whether we're still with the mmap_lock held
 *
 * Returns either number of pages pinned (which may be less than the
 * number requested), or an error. Details about the return value:
 *
 * -- If nr_pages is 0, returns 0.
 * -- If nr_pages is >0, but no pages were pinned, returns -errno.
 * -- If nr_pages is >0, and some pages were pinned, returns the number of
 *    pages pinned. Again, this may be less than nr_pages.
 * -- 0 return value is possible when the fault would need to be retried.
 *
 * The caller is responsible for releasing returned @pages, via put_page().
 *
 * @vmas are valid only as long as mmap_lock is held.
 *
 * Must be called with mmap_lock held.  It may be released.  See below.
 *
 * __get_user_pages walks a process's page tables and takes a reference to
 * each struct page that each user address corresponds to at a given
 * instant. That is, it takes the page that would be accessed if a user
 * thread accesses the given user virtual address at that instant.
 *
 * This does not guarantee that the page exists in the user mappings when
 * __get_user_pages returns, and there may even be a completely different
 * page there in some cases (eg. if mmapped pagecache has been invalidated
 * and subsequently re faulted). However it does guarantee that the page
 * won't be freed completely. And mostly callers simply care that the page
 * contains data that was valid *at some point in time*. Typically, an IO
 * or similar operation cannot guarantee anything stronger anyway because
 * locks can't be held over the syscall boundary.
 *
 * If @gup_flags & FOLL_WRITE == 0, the page must not be written to. If
 * the page is written to, set_page_dirty (or set_page_dirty_lock, as
 * appropriate) must be called after the page is finished with, and
 * before put_page is called.
 *
 * If @locked != NULL, *@locked will be set to 0 when mmap_lock is
 * released by an up_read().  That can happen if @gup_flags does not
 * have FOLL_NOWAIT.
 *
 * A caller using such a combination of @locked and @gup_flags
 * must therefore hold the mmap_lock for reading only, and recognize
 * when it's been released.  Otherwise, it must be held for either
 * reading or writing and will not be released.
 *
 * In most cases, get_user_pages or get_user_pages_fast should be used
 * instead of __get_user_pages. __get_user_pages should be used only if
 * you need some special @gup_flags.
 */
long
__get_user_pages(struct mm_struct *mm,
                 unsigned long start, unsigned long nr_pages,
                 unsigned int gup_flags, struct page **pages,
                 struct vm_area_struct **vmas, int *locked)
{
    long ret = 0, i = 0;
    struct vm_area_struct *vma = NULL;

    if (!nr_pages)
        return 0;

    start = untagged_addr(start);

    BUG_ON(!!pages != !!(gup_flags & (FOLL_GET | FOLL_PIN)));

    do {
        struct page *page;
        unsigned int foll_flags = gup_flags;
        unsigned int page_increm;

        /* first iteration or cross vma bound */
        if (!vma || start >= vma->vm_end) {
            vma = find_extend_vma(mm, start);
            if (!vma)
                panic("find error!");
        }

 retry:
        pr_debug("%s: start(%lx) vma(%lx, %lx)\n",
               __func__, start, vma->vm_start, vma->vm_end);

        page = follow_page_mask(vma, start, foll_flags);
        if (!page) {
            ret = faultin_page(vma, start, &foll_flags, locked);
            switch (ret) {
            case 0:
                goto retry;
            default:
                panic("can not faultin page!");
            }
        } else if (PTR_ERR(page) == -EEXIST) {
            /*
             * Proper page table entry exists, but no corresponding
             * struct page.
             */
            goto next_page;
        } else if (IS_ERR(page)) {
            panic("follow page error: %ld", PTR_ERR(page));
        }

        if (pages) {
            pages[i] = page;
        }
 next_page:
        if (vmas)
            vmas[i] = vma;

        page_increm = 1 + (~(start >> PAGE_SHIFT));
        if (page_increm > nr_pages)
            page_increm = nr_pages;
        i += page_increm;
        start += page_increm * PAGE_SIZE;
        nr_pages -= page_increm;
    } while (nr_pages);

    pr_debug("%s: !(%ld) nr_pages(%lu)\n", __func__, i, nr_pages);
    return i ? i : ret;
}
EXPORT_SYMBOL(__get_user_pages);

static inline long
__get_user_pages_locked(struct mm_struct *mm,
                        unsigned long start,
                        unsigned long nr_pages,
                        struct page **pages,
                        struct vm_area_struct **vmas,
                        int *locked,
                        unsigned int flags)
{
    long ret, pages_done;
    bool lock_dropped;

    if (locked) {
        /* if VM_FAULT_RETRY can be returned, vmas become invalid */
        BUG_ON(vmas);
        /* check caller initialized locked */
        BUG_ON(*locked != 1);
    }

    /*
     * FOLL_PIN and FOLL_GET are mutually exclusive. Traditional behavior
     * is to set FOLL_GET if the caller wants pages[] filled in (but has
     * carelessly failed to specify FOLL_GET), so keep doing that, but only
     * for FOLL_GET, not for the newer FOLL_PIN.
     *
     * FOLL_PIN always expects pages to be non-null, but no need to assert
     * that here, as any failures will be obvious enough.
     */
    if (pages && !(flags & FOLL_PIN))
        flags |= FOLL_GET;

    pages_done = 0;
    lock_dropped = false;
    for (;;) {
        ret = __get_user_pages(mm, start, nr_pages, flags, pages,
                               vmas, locked);
        if (!locked)
            /* VM_FAULT_RETRY couldn't trigger, bypass */
            return ret;

        panic("%s: 1", __func__);
    }

    panic("%s: !", __func__);
}

static long
__get_user_pages_remote(struct mm_struct *mm,
                        unsigned long start, unsigned long nr_pages,
                        unsigned int gup_flags, struct page **pages,
                        struct vm_area_struct **vmas, int *locked)
{
    /*
     * Parts of FOLL_LONGTERM behavior are incompatible with
     * FAULT_FLAG_ALLOW_RETRY because of the FS DAX check requirement on
     * vmas. However, this only comes up if locked is set, and there are
     * callers that do request FOLL_LONGTERM, but do not set locked. So,
     * allow what we can.
     */
    if (gup_flags & FOLL_LONGTERM)
        panic("not support FOLL_LONGTERM!");

    return __get_user_pages_locked(mm, start, nr_pages, pages, vmas,
                                   locked,
                                   gup_flags | FOLL_TOUCH | FOLL_REMOTE);
}

long get_user_pages_remote(struct mm_struct *mm,
                           unsigned long start, unsigned long nr_pages,
                           unsigned int gup_flags, struct page **pages,
                           struct vm_area_struct **vmas, int *locked)
{
    /*
     * FOLL_PIN must only be set internally by the pin_user_pages*() APIs,
     * never directly by the caller, so enforce that with an assertion:
     */
    BUG_ON(gup_flags & FOLL_PIN);

    return __get_user_pages_remote(mm, start, nr_pages, gup_flags,
                                   pages, vmas, locked);
}
EXPORT_SYMBOL(get_user_pages_remote);

int
init_module(void)
{
    printk("module[gup]: init begin ...\n");
    __get_user_pages_cb = __get_user_pages;
    printk("module[gup]: init end!\n");

    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <errno.h>
#include <export.h>
#include <ptrace.h>
#include <signal.h>
#include <current.h>
#include <highmem.h>
#include <pagemap.h>
#include <pgalloc.h>
#include <pgtable.h>
#include <syscalls.h>

static unsigned long fault_around_bytes = rounddown_pow_of_two(65536);

/*
 * Allocate page middle directory.
 * We've already handled the fast-path in-line.
 */
int __pmd_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address)
{
    pmd_t *new = pmd_alloc_one(mm, address);
    if (!new)
        return -ENOMEM;

    if (!pgd_present(*pgd))
        pgd_populate(mm, pgd, new);
    return 0;
}
EXPORT_SYMBOL(__pmd_alloc);

int __pte_alloc(struct mm_struct *mm, pmd_t *pmd)
{
    pgtable_t new = pte_alloc_one(mm);
    if (!new)
        return -ENOMEM;

    if (likely(pmd_none(*pmd))) {   /* Has another populated it ? */
        pmd_populate(mm, pmd, new);
        new = NULL;
    }
    if (new)
        panic("set pte into pmd error!");
    return 0;
}

static vm_fault_t do_anonymous_page(struct vm_fault *vmf)
{
    pte_t entry;
    struct page *page;
    struct vm_area_struct *vma = vmf->vma;

    printk("%s: addr(%lx) pgoff(%lx) flags(%x)\n",
           __func__, vmf->address, vmf->pgoff, vmf->flags);

    if (pte_alloc(vma->vm_mm, vmf->pmd))
        return VM_FAULT_OOM;

    page = alloc_zeroed_user_highpage_movable(vma, vmf->address);
    if (!page)
        panic("out of memory!");

    __SetPageUptodate(page);

    entry = mk_pte(page, vma->vm_page_prot);
    entry = pte_sw_mkyoung(entry);

    vmf->pte = pte_offset_map_lock(vma->vm_mm, vmf->pmd, vmf->address);
    if (!pte_none(*vmf->pte))
        panic("bad pte!");

    set_pte_at(vma->vm_mm, vmf->address, vmf->pte, entry);
    return 0;
}

static vm_fault_t do_fault_around(struct vm_fault *vmf)
{
    int off;
    pgoff_t end_pgoff;
    vm_fault_t ret = 0;
    pgoff_t start_pgoff = vmf->pgoff;
    unsigned long address = vmf->address, nr_pages, mask;

    nr_pages = READ_ONCE(fault_around_bytes) >> PAGE_SHIFT;
    mask = ~(nr_pages * PAGE_SIZE - 1) & PAGE_MASK;

    vmf->address = max(address & mask, vmf->vma->vm_start);
    off = ((address - vmf->address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
    start_pgoff -= off;

    /*
     *  end_pgoff is either the end of the page table, the end of
     *  the vma or nr_pages from start_pgoff, depending what is nearest.
     */
    end_pgoff = start_pgoff -
        ((vmf->address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1)) +
        PTRS_PER_PTE - 1;
    end_pgoff = min3(end_pgoff,
                     vma_pages(vmf->vma) + vmf->vma->vm_pgoff - 1,
                     start_pgoff + nr_pages - 1);

    if (pmd_none(*vmf->pmd)) {
        vmf->prealloc_pte = pte_alloc_one(vmf->vma->vm_mm);
        if (!vmf->prealloc_pte)
            panic("bad memory!");
    }

    vmf->vma->vm_ops->map_pages(vmf, start_pgoff, end_pgoff);

    /* ->map_pages() haven't done anything useful. Cold page cache? */
    if (!vmf->pte)
        panic("no pte!");

    /* check if the page fault is solved */
    vmf->pte -= (vmf->address >> PAGE_SHIFT) - (address >> PAGE_SHIFT);
    if (!pte_none(*vmf->pte))
        ret = VM_FAULT_NOPAGE;

    vmf->address = address;
    vmf->pte = NULL;
    return ret;
}

static vm_fault_t __do_fault(struct vm_fault *vmf)
{
    vm_fault_t ret;
    struct vm_area_struct *vma = vmf->vma;

    if (pmd_none(*vmf->pmd) && !vmf->prealloc_pte) {
        vmf->prealloc_pte = pte_alloc_one(vmf->vma->vm_mm);
        if (!vmf->prealloc_pte)
            return VM_FAULT_OOM;
    }

    ret = vma->vm_ops->fault(vmf);
    if (unlikely(ret & (VM_FAULT_ERROR | VM_FAULT_NOPAGE | VM_FAULT_RETRY |
                        VM_FAULT_DONE_COW)))
        return ret;

    return ret;
}

vm_fault_t finish_fault(struct vm_fault *vmf)
{
    struct page *page;
    vm_fault_t ret = 0;

    /* Did we COW the page? */
    if ((vmf->flags & FAULT_FLAG_WRITE) &&
        !(vmf->vma->vm_flags & VM_SHARED))
        page = vmf->cow_page;
    else
        page = vmf->page;

    /*
     * check even for read faults because we might have lost our CoWed
     * page
     */
    if (!(vmf->vma->vm_flags & VM_SHARED))
        ret = 0;
    if (!ret)
        ret = alloc_set_pte(vmf, page);
    return ret;
}

static vm_fault_t do_read_fault(struct vm_fault *vmf)
{
    vm_fault_t ret = 0;
    struct vm_area_struct *vma = vmf->vma;

    printk("%s: addr(%lx) pgoff(%lx) flags(%x)\n",
           __func__, vmf->address, vmf->pgoff, vmf->flags);

    /*
     * Let's call ->map_pages() first and use ->fault() as fallback
     * if page by the offset is not ready to be mapped (cold cache or
     * something).
     */
    if (vma->vm_ops->map_pages && fault_around_bytes >> PAGE_SHIFT > 1) {
        ret = do_fault_around(vmf);
        if (ret)
            return ret;
    }

    ret = __do_fault(vmf);
    if (unlikely(ret & (VM_FAULT_ERROR | VM_FAULT_NOPAGE | VM_FAULT_RETRY)))
        return ret;

    ret |= finish_fault(vmf);
    return ret;
}

static vm_fault_t do_cow_fault(struct vm_fault *vmf)
{
    vm_fault_t ret;
    struct vm_area_struct *vma = vmf->vma;

    printk("%s: addr(%lx) pgoff(%lx) flags(%x)\n",
           __func__, vmf->address, vmf->pgoff, vmf->flags);

    vmf->cow_page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, vmf->address);
    if (!vmf->cow_page)
        return VM_FAULT_OOM;

    ret = __do_fault(vmf);
    if (unlikely(ret & (VM_FAULT_ERROR | VM_FAULT_NOPAGE | VM_FAULT_RETRY)))
        return ret;
    if (ret & VM_FAULT_DONE_COW)
        return ret;

    copy_user_highpage(vmf->cow_page, vmf->page, vmf->address, vma);
    __SetPageUptodate(vmf->cow_page);

    ret |= finish_fault(vmf);
    if (unlikely(ret & (VM_FAULT_ERROR | VM_FAULT_NOPAGE | VM_FAULT_RETRY)))
        panic("uncharge out!");

    return ret;
}

static vm_fault_t do_fault(struct vm_fault *vmf)
{
    vm_fault_t ret;
    struct vm_area_struct *vma = vmf->vma;

    if (!vma->vm_ops->fault) {
        panic("no fault func!");
    } else if (!(vmf->flags & FAULT_FLAG_WRITE)) {
        ret = do_read_fault(vmf);
    } else if (!(vma->vm_flags & VM_SHARED)) {
        ret = do_cow_fault(vmf);
    } else {
        panic("shared fault!");
    }

    /* preallocated pagetable is unused: free it */
    if (vmf->prealloc_pte) {
        vmf->prealloc_pte = NULL;
    }
    return ret;
}

static vm_fault_t handle_pte_fault(struct vm_fault *vmf)
{
    if (unlikely(pmd_none(*vmf->pmd))) {
        /*
         * Leave __pte_alloc() until later: because vm_ops->fault may
         * want to allocate huge page, and if we expose page table
         * for an instant, it will be difficult to retract from
         * concurrent faults and from rmap lookups.
         */
        vmf->pte = NULL;
    } else {
        /*
         * A regular pmd is established and it can't morph into a huge
         * pmd from under us anymore at this point because we hold the
         * mmap_lock read mode and khugepaged takes it in write mode.
         * So now it's safe to run pte_offset_map().
         */
        vmf->pte = pte_offset_map(vmf->pmd, vmf->address);
        vmf->orig_pte = *vmf->pte;

        /*
         * some architectures can have larger ptes than wordsize,
         * e.g.ppc44x-defconfig has CONFIG_PTE_64BIT=y and
         * CONFIG_32BIT=y, so READ_ONCE cannot guarantee atomic
         * accesses.  The code below just needs a consistent view
         * for the ifs and we later double check anyway with the
         * ptl lock held. So here a barrier will do.
         */
        if (pte_none(vmf->orig_pte))
            vmf->pte = NULL;
    }

    if (!vmf->pte) {
        if (vma_is_anonymous(vmf->vma))
            return do_anonymous_page(vmf);
        else
            return do_fault(vmf);
    }

    panic("%s: !", __func__);
}

static vm_fault_t
__handle_mm_fault(struct vm_area_struct *vma,
                  unsigned long address, unsigned int flags)
{
    pgd_t *pgd;
    struct mm_struct *mm = vma->vm_mm;
    struct vm_fault vmf = {
        .vma = vma,
        .address = address & PAGE_MASK,
        .flags = flags,
        .pgoff = linear_page_index(vma, address),
    };

    pgd = pgd_offset(mm, address);

    vmf.pmd = pmd_alloc(mm, pgd, address);
    if (!vmf.pmd)
        return VM_FAULT_OOM;
    return handle_pte_fault(&vmf);
}

vm_fault_t
_handle_mm_fault(struct vm_area_struct *vma, unsigned long address,
                 unsigned int flags, struct pt_regs *regs)
{
    return __handle_mm_fault(vma, address, flags);
}

struct page *
vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
               pte_t pte)
{
    unsigned long pfn = pte_pfn(pte);

    return pfn_to_page(pfn);
}
EXPORT_SYMBOL(vm_normal_page);

static vm_fault_t pte_alloc_one_map(struct vm_fault *vmf)
{
    struct vm_area_struct *vma = vmf->vma;

    if (!pmd_none(*vmf->pmd))
        goto map_pte;
    if (vmf->prealloc_pte) {
        if (unlikely(!pmd_none(*vmf->pmd)))
            goto map_pte;

        pmd_populate(vma->vm_mm, vmf->pmd, vmf->prealloc_pte);
        vmf->prealloc_pte = NULL;
    } else if (unlikely(pte_alloc(vma->vm_mm, vmf->pmd))) {
        return VM_FAULT_OOM;
    }

 map_pte:

    vmf->pte = pte_offset_map_lock(vma->vm_mm, vmf->pmd, vmf->address);
    return 0;
}

void page_add_file_rmap(struct page *page, bool compound)
{
    /* Todo: */
}

vm_fault_t alloc_set_pte(struct vm_fault *vmf, struct page *page)
{
    pte_t entry;
    vm_fault_t ret;
    struct vm_area_struct *vma = vmf->vma;
    bool write = vmf->flags & FAULT_FLAG_WRITE;

    if (!vmf->pte) {
        ret = pte_alloc_one_map(vmf);
        if (ret)
            return ret;
    }

    if (unlikely(!pte_none(*vmf->pte)))
        return VM_FAULT_NOPAGE;

    entry = mk_pte(page, vma->vm_page_prot);
    entry = pte_sw_mkyoung(entry);
    if (write)
        entry = maybe_mkwrite(pte_mkdirty(entry), vma);
    /* copy-on-write page */
    if (write && !(vma->vm_flags & VM_SHARED)) {
        /* Todo: */
        /*
        page_add_new_anon_rmap(page, vma, vmf->address, false);
        lru_cache_add_inactive_or_unevictable(page, vma);
        */
    } else {
        page_add_file_rmap(page, false);
    }
    set_pte_at(vma->vm_mm, vmf->address, vmf->pte, entry);
    return 0;
}
EXPORT_SYMBOL(alloc_set_pte);

long _do_sys_brk(unsigned long brk)
{
    bool populate;
    unsigned long retval;
    unsigned long min_brk;
    struct vm_area_struct *next;
    unsigned long newbrk, oldbrk, origbrk;
    struct mm_struct *mm = current->mm;
    LIST_HEAD(uf);

    origbrk = mm->brk;

    min_brk = mm->end_data;

    printk("%s 1: brk(%lx) min_brk(%lx) origbrk(%lx)\n",
           __func__, brk, min_brk, origbrk);

    if (brk < min_brk)
        goto out;

    /*
     * Check against rlimit here. If this check is done later after the test
     * of oldbrk with newbrk then it can escape the test and let the data
     * segment grow beyond its set limit the in case where the limit is
     * not page aligned -Ram Gupta
     */
    if (check_data_rlimit(rlimit(RLIMIT_DATA), brk, mm->start_brk,
                          mm->end_data, mm->start_data))
        goto out;

    newbrk = PAGE_ALIGN(brk);
    oldbrk = PAGE_ALIGN(mm->brk);
    if (oldbrk == newbrk) {
        mm->brk = brk;
        goto success;
    }

    /*
     * Always allow shrinking brk.
     * __do_munmap() may downgrade mmap_lock to read.
     */
    if (brk <= mm->brk)
        panic("brk <= mm->brk");

    /* Check against existing mmap mappings. */
    next = find_vma(mm, oldbrk);

    printk("%s 2: brk(%lx) min_brk(%lx) next(%lx) newbrk(%lx)\n",
           __func__, brk, min_brk, next, newbrk);

    if (next && newbrk + PAGE_SIZE > vm_start_gap(next))
        goto out;

    /* Ok, looks good - let it rip. */
    if (do_brk_flags(oldbrk, newbrk-oldbrk, 0, &uf) < 0)
        goto out;
    mm->brk = brk;

success:
    populate = newbrk > oldbrk && (mm->def_flags & VM_LOCKED) != 0;
    if (populate)
        mm_populate(oldbrk, newbrk - oldbrk);
    return brk;

out:
    retval = origbrk;
    return retval;
}

static int
init_module(void)
{
    printk("module[pgalloc]: init begin ...\n");

    handle_mm_fault = _handle_mm_fault;
    do_sys_brk = _do_sys_brk;

    printk("module[pgalloc]: init end!\n");

    return 0;
}

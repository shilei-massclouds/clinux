// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <bug.h>
#include <csr.h>
#include <export.h>
#include <fixmap.h>
#include <kernel.h>
#include <signal.h>
#include <string.h>
#include <current.h>
#include <pgtable.h>
#include <mm_types.h>

void init_mprotect(void);

static phys_alloc_t phys_alloc_fn;

extern do_page_fault_t do_page_fault_func;

void *dtb_early_va;
EXPORT_SYMBOL(dtb_early_va);

handle_mm_fault_t handle_mm_fault;
EXPORT_SYMBOL(handle_mm_fault);

struct mm_struct init_mm = {
    .pgd    = swapper_pg_dir,
};
EXPORT_SYMBOL(init_mm);

void
setup_fixmap_pgd(void)
{
    uintptr_t va, end_va;
    uintptr_t pgd_idx = pgd_index(FIXADDR_START);
    uintptr_t pmd_idx = pmd_index(FIXADDR_START);

    BUG_ON(!pgd_none(early_pgd[pgd_idx]));
    BUG_ON(!pmd_none(fixmap_pmd[pmd_idx]));

    early_pgd[pgd_idx] =
        pfn_pgd(PFN_DOWN(__pa(fixmap_pmd)), PAGE_TABLE);

    fixmap_pmd[pmd_idx] =
        pfn_pmd(PFN_DOWN(__pa(fixmap_pt)), PAGE_TABLE);

    end_va = fix_to_virt(FIX_FDT) + FIX_FDT_SIZE;
    for (va = fix_to_virt(FIX_FDT); va < end_va; va += PAGE_SIZE) {
        uintptr_t pte_idx = pte_index(va);
        uintptr_t pa = dtb_early_pa + (va - fix_to_virt(FIX_FDT));
        fixmap_pt[pte_idx] = pfn_pte(PFN_DOWN(pa), PAGE_KERNEL);
    }

    dtb_early_va = (void *)fix_to_virt(FIX_FDT) + (dtb_early_pa & ~PAGE_MASK);
}
EXPORT_SYMBOL(setup_fixmap_pgd);

void
__set_fixmap(enum fixed_addresses idx, phys_addr_t phys, pgprot_t prot)
{
    unsigned long addr = fix_to_virt(idx);
    pte_t *ptep;

    BUG_ON(idx <= FIX_HOLE || idx >= __end_of_fixed_addresses);

    ptep = &fixmap_pt[pte_index(addr)];

    if (pgprot_val(prot)) {
        set_pte(ptep, pfn_pte(phys >> PAGE_SHIFT, prot));
    } else {
        set_pte(ptep, __pte(0));
        local_flush_tlb_page(addr);
    }
}

static pmd_t *
get_pmd_virt(phys_addr_t pa)
{
    clear_fixmap(FIX_PMD);
    return (pmd_t *)set_fixmap_offset(FIX_PMD, pa);
}

static phys_addr_t
alloc_pmd(uintptr_t va)
{
	uintptr_t pmd_num;

    BUG_ON(!phys_alloc_fn);
    return phys_alloc_fn(PAGE_SIZE, PAGE_SIZE);
}

static void
create_pmd_mapping(pmd_t *pmdp,
                   uintptr_t va, phys_addr_t pa,
                   phys_addr_t sz, pgprot_t prot)
{
	uintptr_t pmd_idx = pmd_index(va);

    BUG_ON(sz != PMD_SIZE);

    if (pmd_none(pmdp[pmd_idx]))
        pmdp[pmd_idx] = pfn_pmd(PFN_DOWN(pa), prot);
}

static void
create_pgd_mapping(pgd_t *pgdp,
                   uintptr_t va, phys_addr_t pa,
                   phys_addr_t sz, pgprot_t prot)
{
	pmd_t *nextp;
	phys_addr_t next_phys;
	uintptr_t pgd_idx = pgd_index(va);

	if (sz == PGDIR_SIZE) {
		if (pgd_none(pgdp[pgd_idx]))
			pgdp[pgd_idx] = pfn_pgd(PFN_DOWN(pa), prot);
		return;
	}

	if (pgd_none(pgdp[pgd_idx])) {
		next_phys = alloc_pmd(va);
		pgdp[pgd_idx] = pfn_pgd(PFN_DOWN(next_phys), PAGE_TABLE);
		nextp = get_pmd_virt(next_phys);
		memset(nextp, 0, PAGE_SIZE);
    } else {
		next_phys = PFN_PHYS(pgd_pfn(pgdp[pgd_idx]));
		nextp = get_pmd_virt(next_phys);
    }

	create_pmd_mapping(nextp, va, pa, sz, prot);
}

void
setup_vm_final(struct memblock_region *regions,
               unsigned long regions_cnt,
               phys_alloc_t alloc)
{
	uintptr_t va;
	phys_addr_t pa, start, end;
	struct memblock_region *reg;

    phys_alloc_fn = alloc;

	/* Setup swapper PGD for fixmap */
	create_pgd_mapping(swapper_pg_dir,
                       FIXADDR_START, __pa(fixmap_pmd),
                       PGDIR_SIZE, PAGE_TABLE);

	/* Map all memory banks */
    for (reg = regions; reg < (regions + regions_cnt); reg++) {
        start = reg->base;
        end = start + reg->size;

        if (start >= end)
            break;

        for (pa = start; pa < end; pa += PMD_SIZE) {
            va = (uintptr_t)__va(pa);
            //printk("%s: va(%p) pa(%p)\n", __func__, va, pa);
            create_pgd_mapping(swapper_pg_dir, va, pa,
                               PMD_SIZE, PAGE_KERNEL_EXEC);
        }
    }

    /* Clear fixmap PTE and PMD mappings */
    clear_fixmap(FIX_PTE);
    clear_fixmap(FIX_PMD);

    /* Move to swapper page table */
    csr_write(CSR_SATP, PFN_DOWN(__pa(swapper_pg_dir)) | SATP_MODE);
    local_flush_tlb_all();
}
EXPORT_SYMBOL(setup_vm_final);

void _do_page_fault(struct pt_regs *regs)
{
    struct mm_struct *mm;
    struct task_struct *tsk;
    struct vm_area_struct *vma;
    unsigned long addr, cause;
    vm_fault_t fault;
    int code = SEGV_MAPERR;
    unsigned int flags = FAULT_FLAG_DEFAULT;

    cause = regs->cause;
    addr = regs->badaddr;

    printk("--- --- %s: cause(%lx) addr(%ld)\n",
           __func__, cause, addr);

    tsk = current;
    mm = tsk->mm;

    if (user_mode(regs))
        flags |= FAULT_FLAG_USER;

 retry:
    printk("--- --- %s: step0 mm(%p)\n", __func__, mm);

    vma = find_vma(mm, addr);
    if (unlikely(!vma))
        panic("bad area!");
    printk("--- --- %s: step2\n", __func__);
    if (likely(vma->vm_start <= addr))
        goto good_area;
    if (unlikely(!(vma->vm_flags & VM_GROWSDOWN)))
        panic("target(%lx) nearest(%lx) epc(%lx) cause(%lx) bad vm_flags!",
              addr, vma->vm_start, regs->epc, cause);
    if (unlikely(expand_stack(vma, addr)))
        panic("expand stack error!");

    /*
     * Ok, we have a good vm_area for this memory access, so
     * we can handle it.
     */
 good_area:
    code = SEGV_ACCERR;

    switch (cause) {
    case EXC_INST_PAGE_FAULT:
        if (!(vma->vm_flags & VM_EXEC))
            panic("bad vm_flags!");
        break;
    case EXC_LOAD_PAGE_FAULT:
        if (!(vma->vm_flags & VM_READ))
            panic("bad vm_flags!");
        break;
    case EXC_STORE_PAGE_FAULT:
        if (!(vma->vm_flags & VM_WRITE))
            panic("bad vm_flags!");
        flags |= FAULT_FLAG_WRITE;
        break;
    default:
        panic("%s: unhandled cause %lu", __func__, cause);
    }

    /*
     * If for any reason at all we could not handle the fault,
     * make sure we exit gracefully rather than endlessly redo
     * the fault.
     */
    fault = handle_mm_fault(vma, addr, flags, regs);

    if (unlikely(fault & VM_FAULT_ERROR)) {
        if (fault & VM_FAULT_OOM)
            panic("out_of_memory");
        else if (fault & VM_FAULT_SIGBUS)
            panic("do_sigbus");
        BUG();
    }

    if (flags & FAULT_FLAG_ALLOW_RETRY) {
        if (fault & VM_FAULT_RETRY) {
            flags |= FAULT_FLAG_TRIED;

            /*
             * No need to mmap_read_unlock(mm) as we would
             * have already released it in __lock_page_or_retry
             * in mm/filemap.c.
             */
            goto retry;
        }
    }

    //printk("--- --- %s: flags(%x)\n", __func__, flags);
    //panic("%s: !", __func__);
    return;
}

int
init_module(void)
{
    printk("module[mm]: init begin ...\n");

    init_mprotect();

    do_page_fault_func = _do_page_fault;

    setup_fixmap_pgd();

    printk("module[mm]: init end!\n");

    return 0;
}

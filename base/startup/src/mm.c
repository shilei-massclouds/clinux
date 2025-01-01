// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <page.h>
#include <export.h>
#include <linkage.h>
#include <pgtable.h>
#include <uaccess.h>

#ifndef __riscv_cmodel_medany
#error "Don't use absolute addressing now."
#endif

uintptr_t kernel_start = 0;
EXPORT_SYMBOL(kernel_start);

pgd_t early_pgd[PTRS_PER_PGD] __initdata __aligned(PAGE_SIZE);
EXPORT_SYMBOL(early_pgd);
pmd_t early_pmd[PTRS_PER_PMD] __initdata __aligned(PAGE_SIZE);
EXPORT_SYMBOL(early_pmd);
pmd_t fixmap_pmd[PTRS_PER_PMD] __page_aligned_bss;
EXPORT_SYMBOL(fixmap_pmd);
pte_t fixmap_pt[PTRS_PER_PTE] __page_aligned_bss;
EXPORT_SYMBOL(fixmap_pt);
pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned_bss;
EXPORT_SYMBOL(swapper_pg_dir);

unsigned long pfn_base;
EXPORT_SYMBOL(pfn_base);

unsigned long va_pa_offset;
EXPORT_SYMBOL(va_pa_offset);

phys_addr_t dtb_early_pa __initdata;
EXPORT_SYMBOL(dtb_early_pa);

do_page_fault_t do_page_fault_func;
EXPORT_SYMBOL(do_page_fault_func);

extern char _start[];

void setup_vm_early(uintptr_t dtb_pa)
{
    uintptr_t load_pa = (uintptr_t)(&_start);
    uintptr_t pgd_idx = pgd_index(PAGE_OFFSET);
    uintptr_t pmd_idx = pmd_index(PAGE_OFFSET);

    va_pa_offset = PAGE_OFFSET - load_pa;
    pfn_base = PFN_DOWN(load_pa);

    early_pgd[pgd_idx] =
        pfn_pgd(PFN_DOWN((uintptr_t)early_pmd), PAGE_TABLE);

    early_pmd[pmd_idx] =
        pfn_pmd(PFN_DOWN(load_pa), PAGE_KERNEL_EXEC);

    early_pmd[pmd_idx + 1] =
        pfn_pmd(PFN_DOWN(load_pa + PMD_SIZE), PAGE_KERNEL_EXEC);

    /* Qemu pflash acts as the repository of modules,
     * startup loads modules from it.
     * The pflash is located at 0x22000000 in PA,
     * just setup identity-mapping for the first pgdir temporily. */
    early_pgd[0] = pfn_pgd(PFN_DOWN(0), PAGE_KERNEL);

    dtb_early_pa = dtb_pa;

    kernel_start = load_pa;
}

extern unsigned long
__asm_copy_to_user(void *to, const void *from, unsigned long n);

unsigned long
asm_copy_to_user(void *to, const void *from, unsigned long n)
{
    return __asm_copy_to_user(to, from, n);
}
EXPORT_SYMBOL(asm_copy_to_user);

extern unsigned long
__asm_copy_from_user(void *to, const void *from, unsigned long n);

unsigned long
asm_copy_from_user(void *to, const void *from, unsigned long n)
{
    return __asm_copy_from_user(to, from, n);
}
EXPORT_SYMBOL(asm_copy_from_user);

extern unsigned long
__clear_user(void *addr, unsigned long n);
EXPORT_SYMBOL(__clear_user);

void do_page_fault(struct pt_regs *regs)
{
    do_page_fault_func(regs);
}

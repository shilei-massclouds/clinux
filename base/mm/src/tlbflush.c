// SPDX-License-Identifier: GPL-2.0

#include <mm.h>
#include <sbi.h>
#include <export.h>

/*
 * This function must not be called with cmask being null.
 * Kernel may panic if cmask is NULL.
 */
static void
__sbi_tlb_flush_range(unsigned long start, unsigned long size)
{
    /* local cpu is the only cpu present in cpumask */
    if (size <= PAGE_SIZE)
        local_flush_tlb_page(start);
    else
        local_flush_tlb_all();
}

void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
    __sbi_tlb_flush_range(addr, PAGE_SIZE);
}
EXPORT_SYMBOL(flush_tlb_page);

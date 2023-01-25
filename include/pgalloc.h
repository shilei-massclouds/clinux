/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2009 Chen Liqin <liqin.chen@sunplusct.com>
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_PGALLOC_H
#define _ASM_RISCV_PGALLOC_H

#include <string.h>
#include <pgtable.h>
#include <mm_types.h>

static inline pgd_t *pgd_alloc(struct mm_struct *mm)
{
    pgd_t *pgd;

    pgd = (pgd_t *)__get_free_page(GFP_KERNEL);
    if (likely(pgd != NULL)) {
        memset(pgd, 0, USER_PTRS_PER_PGD * sizeof(pgd_t));
        /* Copy kernel mappings */
        memcpy(pgd + USER_PTRS_PER_PGD,
               init_mm.pgd + USER_PTRS_PER_PGD,
               (PTRS_PER_PGD - USER_PTRS_PER_PGD) * sizeof(pgd_t));
    }
    return pgd;
}

static inline pmd_t *
pmd_alloc_one(struct mm_struct *mm, unsigned long addr)
{
    struct page *page;
    gfp_t gfp = GFP_PGTABLE_USER;

    if (mm == &init_mm)
        gfp = GFP_PGTABLE_KERNEL;
    page = alloc_pages(gfp, 0);
    if (!page)
        return NULL;
    return (pmd_t *)page_address(page);
}

static inline bool pgtable_pte_page_ctor(struct page *page)
{
    __SetPageTable(page);
    return true;
}

static inline pgtable_t __pte_alloc_one(struct mm_struct *mm, gfp_t gfp)
{
    struct page *pte;

    pte = alloc_page(gfp);
    if (!pte)
        return NULL;
    if (!pgtable_pte_page_ctor(pte)) {
        panic("set pte error!");
        return NULL;
    }

    return pte;
}

/**
 * pte_alloc_one - allocate a page for PTE-level user page table
 * @mm: the mm_struct of the current context
 *
 * Allocates a page and runs the pgtable_pte_page_ctor().
 *
 * Return: `struct page` initialized as page table or %NULL on error
 */
static inline pgtable_t pte_alloc_one(struct mm_struct *mm)
{
    return __pte_alloc_one(mm, GFP_PGTABLE_USER);
}

static inline void
pmd_populate(struct mm_struct *mm, pmd_t *pmd, pgtable_t pte)
{
    unsigned long pfn = virt_to_pfn(page_address(pte));

    set_pmd(pmd, __pmd((pfn << _PAGE_PFN_SHIFT) | _PAGE_TABLE));
}

struct page *
vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
               pte_t pte);

vm_fault_t alloc_set_pte(struct vm_fault *vmf, struct page *page);

static inline pte_t maybe_mkwrite(pte_t pte, struct vm_area_struct *vma)
{
    if (likely(vma->vm_flags & VM_WRITE))
        pte = pte_mkwrite(pte);
    return pte;
}

#endif /* _ASM_RISCV_PGALLOC_H */

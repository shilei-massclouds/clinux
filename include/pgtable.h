/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PGTABLE_H
#define _LINUX_PGTABLE_H

#include <gfp.h>
#include <bits.h>
#include <page.h>
#include <const.h>
#include <sizes.h>

struct mm_struct;
struct vm_area_struct;

#define GFP_PGTABLE_KERNEL  (GFP_KERNEL | __GFP_ZERO)
#define GFP_PGTABLE_USER    (GFP_PGTABLE_KERNEL | __GFP_ACCOUNT)

#define VMALLOC_SIZE    (KERN_VIRT_SIZE >> 1)
#define VMALLOC_END     (PAGE_OFFSET - 1)
#define VMALLOC_START   (PAGE_OFFSET - VMALLOC_SIZE)

/*
 * Roughly size the vmemmap space to be large enough to fit enough
 * struct pages to map half the virtual address space. Then
 * position vmemmap directly below the VMALLOC region.
 */
#define VMEMMAP_SHIFT \
    (CONFIG_VA_BITS - PAGE_SHIFT - 1 + STRUCT_PAGE_MAX_SHIFT)

#define VMEMMAP_SIZE    (_AC(1, UL) << VMEMMAP_SHIFT)
#define VMEMMAP_END     (VMALLOC_START - 1)
#define VMEMMAP_START   (VMALLOC_START - VMEMMAP_SIZE)

#define PCI_IO_SIZE     SZ_16M
#define PCI_IO_END      VMEMMAP_START
#define PCI_IO_START    (PCI_IO_END - PCI_IO_SIZE)

#define FIXADDR_SIZE    PMD_SIZE
#define FIXADDR_TOP     PCI_IO_START
#define FIXADDR_START   (FIXADDR_TOP - FIXADDR_SIZE)

/*
 * Address for flash
 * The flash is used for module
 */
#define FLASH_SIZE      0x0000000002000000UL
#define FLASH_PA        0x0000000020000000UL

#define FLASH_VA \
    _ALIGN_DOWN((FIXADDR_START - FLASH_SIZE - PGDIR_SIZE), FLASH_PA)

#define FLASH_HEAD_SIZE 0x100

#define PAGE_COPY           PAGE_READ
#define PAGE_COPY_EXEC      PAGE_EXEC
#define PAGE_COPY_READ_EXEC PAGE_READ_EXEC
#define PAGE_SHARED         PAGE_WRITE
#define PAGE_SHARED_EXEC    PAGE_WRITE_EXEC

/* MAP_PRIVATE permissions: xwr (copy-on-write) */
#define __P000  PAGE_NONE
#define __P001  PAGE_READ
#define __P010  PAGE_COPY
#define __P011  PAGE_COPY
#define __P100  PAGE_EXEC
#define __P101  PAGE_READ_EXEC
#define __P110  PAGE_COPY_EXEC
#define __P111  PAGE_COPY_READ_EXEC

/* MAP_SHARED permissions: xwr */
#define __S000  PAGE_NONE
#define __S001  PAGE_READ
#define __S010  PAGE_SHARED
#define __S011  PAGE_SHARED
#define __S100  PAGE_EXEC
#define __S101  PAGE_READ_EXEC
#define __S110  PAGE_SHARED_EXEC
#define __S111  PAGE_SHARED_EXEC

#define     __PGTBL_PGD_MODIFIED    0
#define     __PGTBL_P4D_MODIFIED    1
#define     __PGTBL_PUD_MODIFIED    2
#define     __PGTBL_PMD_MODIFIED    3
#define     __PGTBL_PTE_MODIFIED    4

#define     PGTBL_PGD_MODIFIED  BIT(__PGTBL_PGD_MODIFIED)
#define     PGTBL_P4D_MODIFIED  BIT(__PGTBL_P4D_MODIFIED)
#define     PGTBL_PUD_MODIFIED  BIT(__PGTBL_PUD_MODIFIED)
#define     PGTBL_PMD_MODIFIED  BIT(__PGTBL_PMD_MODIFIED)
#define     PGTBL_PTE_MODIFIED  BIT(__PGTBL_PTE_MODIFIED)

#define TASK_SIZE (PGDIR_SIZE * PTRS_PER_PGD / 2)

/* Number of PGD entries that a user-mode program can use */
#define USER_PTRS_PER_PGD   (TASK_SIZE / PGDIR_SIZE)

#define pmd_addr_end(addr, end)                     \
({  unsigned long __boundary = ((addr) + PMD_SIZE) & PMD_MASK;  \
    (__boundary - 1 < (end) - 1)? __boundary: (end);        \
})

typedef unsigned int pgtbl_mod_mask;

static inline void
set_pte(pte_t *ptep, pte_t pteval)
{
    *ptep = pteval;
}

static inline pgd_t *
pgd_offset_pgd(pgd_t *pgd, unsigned long address)
{
    return (pgd + pgd_index(address));
};

static inline unsigned long
pgd_page_vaddr(pgd_t pgd)
{
    return (unsigned long)pfn_to_virt(pgd_val(pgd) >> _PAGE_PFN_SHIFT);
}

static inline pmd_t *
pmd_offset(pgd_t *pgd, unsigned long address)
{
    return (pmd_t *)pgd_page_vaddr(*pgd) + pmd_index(address);
}

/*
 * a shortcut to get a pgd_t in a given mm
 */
#define pgd_offset(mm, address) pgd_offset_pgd((mm)->pgd, (address))

/*
 * a shortcut which implies the use of the kernel's pgd, instead
 * of a process's
 */
#define pgd_offset_k(address)   pgd_offset(&init_mm, (address))

#define pgd_addr_end(addr, end) \
({  unsigned long __boundary = ((addr) + PGDIR_SIZE) & PGDIR_MASK; \
    (__boundary - 1 < (end) - 1)? __boundary: (end);           \
})

static inline int pgd_present(pgd_t pgd)
{
    return (pgd_val(pgd) & _PAGE_PRESENT);
}

static inline void
set_pgd(pgd_t *pgdp, pgd_t pgd)
{
    *pgdp = pgd;
}

static inline void
set_pmd(pmd_t *pmdp, pmd_t pmd)
{
    *pmdp = pmd;
}

static inline void
pgd_populate(struct mm_struct *mm, pgd_t *pgd, pmd_t *pmd)
{
    unsigned long pfn = virt_to_pfn(pmd);
    set_pgd(pgd, __pgd((pfn << _PAGE_PFN_SHIFT) | _PAGE_TABLE));
}

static inline pte_t *
__pte_alloc_one_kernel(struct mm_struct *mm)
{
    return (pte_t *)__get_free_page(GFP_PGTABLE_KERNEL);
}

static inline pte_t *
pte_alloc_one_kernel(struct mm_struct *mm)
{
    return __pte_alloc_one_kernel(mm);
}

static inline void
pmd_populate_kernel(struct mm_struct *mm, pmd_t *pmd, pte_t *pte)
{
    unsigned long pfn = virt_to_pfn(pte);
    set_pmd(pmd, __pmd((pfn << _PAGE_PFN_SHIFT) | _PAGE_TABLE));
}

static inline void
pte_free_kernel(struct mm_struct *mm, pte_t *pte)
{
    free_page((unsigned long)pte);
}

static inline unsigned long
pmd_page_vaddr(pmd_t pmd)
{
    return (unsigned long)pfn_to_virt(pmd_val(pmd) >> _PAGE_PFN_SHIFT);
}

static inline pte_t *
pte_offset_kernel(pmd_t *pmd, unsigned long address)
{
    return (pte_t *)pmd_page_vaddr(*pmd) + pte_index(address);
}

static inline void
set_pte_at(struct mm_struct *mm, unsigned long addr, pte_t *ptep, pte_t pteval)
{
    set_pte(ptep, pteval);
}

#define mk_pte(page, prot)  pfn_pte(page_to_pfn(page), prot)

static inline pte_t pte_sw_mkyoung(pte_t pte)
{
    return pte;
}

static inline int pte_present(pte_t pte)
{
    return (pte_val(pte) & (_PAGE_PRESENT | _PAGE_PROT_NONE));
}

static inline int pmd_present(pmd_t pmd)
{
    return (pmd_val(pmd) & (_PAGE_PRESENT | _PAGE_PROT_NONE));
}

/* Yields the page frame number (PFN) of a page table entry */
static inline unsigned long pte_pfn(pte_t pte)
{
    return (pte_val(pte) >> _PAGE_PFN_SHIFT);
}

static inline int pte_dirty(pte_t pte)
{
    return pte_val(pte) & _PAGE_DIRTY;
}

struct page *
vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
               pte_t pte);

static inline pte_t pte_mkwrite(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_WRITE);
}

static inline pte_t pte_mkdirty(pte_t pte)
{
    return __pte(pte_val(pte) | _PAGE_DIRTY);
}

#endif /* _LINUX_PGTABLE_H */

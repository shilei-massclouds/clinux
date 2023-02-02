// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <vma.h>
#include <errno.h>
#include <export.h>
#include <ioremap.h>
#include <pgalloc.h>
#include <pgtable.h>

static inline pmd_t *
pmd_alloc_track(struct mm_struct *mm, pgd_t *pgd,
                unsigned long address, pgtbl_mod_mask *mod_mask)
{
    if (unlikely(pgd_none(*pgd))) {
        if (__pmd_alloc(mm, pgd, address))
            return NULL;
        *mod_mask |= PGTBL_PGD_MODIFIED;
    }
    return pmd_offset(pgd, address);
}

int
__pte_alloc_kernel(pmd_t *pmd)
{
    pte_t *new = pte_alloc_one_kernel(&init_mm);
    if (!new)
        return -ENOMEM;

    if (likely(pmd_none(*pmd))) {   /* Has another populated it ? */
        pmd_populate_kernel(&init_mm, pmd, new);
        new = NULL;
    }
    if (new)
        pte_free_kernel(&init_mm, new);
    return 0;
}

static inline pte_t *
pte_alloc_kernel_track(pmd_t *pmd,
                       unsigned long address,
                       pgtbl_mod_mask *mod_mask)
{
    if (unlikely(pmd_none(*pmd))) {
        if (__pte_alloc_kernel(pmd))
            return NULL;
        *mod_mask |= PGTBL_PMD_MODIFIED;
    }
    return pte_offset_kernel(pmd, address);
}

static int
ioremap_pte_range(pmd_t *pmd,
                  unsigned long addr, unsigned long end,
                  phys_addr_t phys_addr,
                  pgprot_t prot, pgtbl_mod_mask *mask)
{
    pte_t *pte;
    u64 pfn;

    pfn = phys_addr >> PAGE_SHIFT;
    pte = pte_alloc_kernel_track(pmd, addr, mask);
    if (!pte)
        return -ENOMEM;
    do {
        BUG_ON(!pte_none(*pte));
        set_pte_at(&init_mm, addr, pte, pfn_pte(pfn, prot));
        pfn++;
    } while (pte++, addr += PAGE_SIZE, addr != end);
    *mask |= PGTBL_PTE_MODIFIED;
    return 0;
}

static inline int
ioremap_pmd_range(pgd_t *pgd,
                  unsigned long addr, unsigned long end,
                  phys_addr_t phys_addr,
                  pgprot_t prot, pgtbl_mod_mask *mask)
{
    pmd_t *pmd;
    unsigned long next;

    pmd = pmd_alloc_track(&init_mm, pgd, addr, mask);
    if (!pmd)
        return -ENOMEM;
    do {
        next = pmd_addr_end(addr, end);
        if (ioremap_pte_range(pmd, addr, next, phys_addr, prot, mask))
            return -ENOMEM;
    } while (pmd++, phys_addr += (next - addr), addr = next, addr != end);
    return 0;
}

int
ioremap_page_range(unsigned long addr, unsigned long end,
                   phys_addr_t phys_addr, pgprot_t prot)
{
    int err;
    pgd_t *pgd;
    unsigned long next;
    pgtbl_mod_mask mask = 0;

    BUG_ON(addr >= end);

    pgd = pgd_offset_k(addr);
    do {
        next = pgd_addr_end(addr, end);
        err = ioremap_pmd_range(pgd, addr, next, phys_addr, prot, &mask);
        if (err)
            break;
    } while (pgd++, phys_addr += (next - addr), addr = next, addr != end);

    return err;
}

void *
ioremap_prot(phys_addr_t addr, size_t size, unsigned long prot)
{
    unsigned long offset, vaddr;
    phys_addr_t last_addr;
    struct vm_struct *area;

    /* Disallow wrap-around or zero size */
    last_addr = addr + size - 1;
    if (!size || last_addr < addr)
        return NULL;

    /* Page-align mappings */
    offset = addr & (~PAGE_MASK);
    addr -= offset;
    size = PAGE_ALIGN(size + offset);

    area = get_vm_area(size, VM_IOREMAP);
    if (!area)
        return NULL;
    vaddr = (unsigned long)area->addr;

    if (ioremap_page_range(vaddr, vaddr + size, addr, __pgprot(prot))) {
        free_vm_area(area);
        return NULL;
    }

    return (void *)(vaddr + offset);
}
EXPORT_SYMBOL(ioremap_prot);

static int
init_module(void)
{
    printk("module[ioremap]: init begin ...\n");
    printk("module[ioremap]: init end!\n");
    return 0;
}

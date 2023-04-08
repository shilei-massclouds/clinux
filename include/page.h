/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _PAGE_H
#define _PAGE_H

#include <config.h>
#include <const.h>
#include <types.h>
#include <pgtable-bits.h>

#define __va_to_pa(x)       ((unsigned long)(x) - va_pa_offset)
#define __virt_to_phys(x)   __va_to_pa(x)
#define __pa(x)             __virt_to_phys((unsigned long)(x))
#define virt_to_phys(x)     __pa((unsigned long)x)

#define __pa_to_va(x)   ((void *)((unsigned long) (x) + va_pa_offset))
#define __va(x)         ((void *)__pa_to_va((phys_addr_t)(x)))
#define phys_to_virt(x) __va(x)

#define PAGE_SHIFT	CONFIG_PAGE_SHIFT
#define PAGE_SIZE	(_AC(1, UL) << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE - 1))

#define PAGE_OFFSET _AC(CONFIG_PAGE_OFFSET, UL)

#define KERN_VIRT_SIZE (-PAGE_OFFSET)

#define pfn_valid(pfn) \
    (((pfn) >= ARCH_PFN_OFFSET) && (((pfn) - ARCH_PFN_OFFSET) < max_mapnr))

#define __pfn_to_page(pfn)  (mem_map + ((pfn) - ARCH_PFN_OFFSET))

#define __page_to_pfn(page) \
    ((unsigned long)((page) - mem_map) + ARCH_PFN_OFFSET)

#define PFN_ALIGN(x) (((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)

#define PFN_UP(x)    (((x) + PAGE_SIZE - 1) >> PAGE_SHIFT)
#define PFN_DOWN(x)  ((x) >> PAGE_SHIFT)
#define PFN_PHYS(x)  ((phys_addr_t)(x) << PAGE_SHIFT)
#define PHYS_PFN(x)  ((unsigned long)((x) >> PAGE_SHIFT))

#define phys_to_pfn(phys)   (PFN_DOWN(phys))
#define pfn_to_phys(pfn)    (PFN_PHYS(pfn))
#define page_to_phys(page)  (pfn_to_phys(page_to_pfn(page)))

#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page

#define virt_to_pfn(vaddr)  (phys_to_pfn(__pa(vaddr)))
#define pfn_to_virt(pfn)    (__va(pfn_to_phys(pfn)))

#define virt_to_page(vaddr) (pfn_to_page(virt_to_pfn(vaddr)))
#define page_to_virt(page)  (pfn_to_virt(page_to_pfn(page)))

/*
 * paging: SV-39, va-39bits, pa-56bits
 * root(39) -> pgd(30) -> pmd(21) -> pt(12) -> phy_page(3)
 */

#define PGDIR_SHIFT     30
#define PGDIR_SIZE      (_AC(1, UL) << PGDIR_SHIFT)
#define PGDIR_MASK      (~(PGDIR_SIZE - 1))

#define PTRS_PER_PGD    (PAGE_SIZE / sizeof(pgd_t))
#define pgd_index(a)    (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_val(x)      ((x).pgd)
#define __pgd(x)        ((pgd_t) { (x) })

#define PMD_SHIFT       (21)
#define PMD_SIZE        (_AC(1, UL) << PMD_SHIFT)
#define PMD_MASK        (~(PMD_SIZE - 1))

#define PTRS_PER_PMD    (PAGE_SIZE / sizeof(pmd_t))
#define pmd_index(a)    (((a) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))
#define pmd_val(x)      ((x).pmd)
#define __pmd(x)        ((pmd_t) { (x) })

#define PTRS_PER_PTE    (PAGE_SIZE / sizeof(pte_t))
#define pte_index(a)    (((a) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_val(x)      ((x).pte)
#define __pte(x)        ((pte_t) { (x) })

#define pgprot_val(x)   ((x).pgprot)
#define __pgprot(x)     ((pgprot_t) { (x) })

#define arch_vm_get_page_prot(vm_flags) __pgprot(0)

#define pfn_pgd(pfn, prot) \
    __pgd(((pfn) << _PAGE_PFN_SHIFT) | pgprot_val((prot)))

#define pgd_pfn(pgd) \
    (pgd_val(pgd) >> _PAGE_PFN_SHIFT)

#define pfn_pmd(pfn, prot) \
    __pmd(((pfn) << _PAGE_PFN_SHIFT) | pgprot_val((prot)))

#define pfn_pte(pfn, prot) \
    __pte(((pfn) << _PAGE_PFN_SHIFT) | pgprot_val((prot)))

#define _PAGE_BASE  (_PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_USER)

#define PAGE_NONE       __pgprot(_PAGE_PROT_NONE)
#define PAGE_READ       __pgprot(_PAGE_BASE | _PAGE_READ)
#define PAGE_WRITE      __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_WRITE)
#define PAGE_EXEC       __pgprot(_PAGE_BASE | _PAGE_EXEC)
#define PAGE_READ_EXEC  __pgprot(_PAGE_BASE | _PAGE_READ | _PAGE_EXEC)
#define PAGE_WRITE_EXEC __pgprot(_PAGE_BASE | _PAGE_READ | \
                                 _PAGE_EXEC | _PAGE_WRITE)

#define _PAGE_KERNEL    (_PAGE_READ | _PAGE_WRITE | \
                         _PAGE_PRESENT | _PAGE_ACCESSED | _PAGE_DIRTY)

/*
 * The RISC-V ISA doesn't yet specify how to query or modify PMAs, so we can't
 * change the properties of memory regions.
 */
#define _PAGE_IOREMAP _PAGE_KERNEL

#define PAGE_KERNEL         __pgprot(_PAGE_KERNEL)
#define PAGE_KERNEL_EXEC    __pgprot(_PAGE_KERNEL | _PAGE_EXEC)
#define PAGE_TABLE          __pgprot(_PAGE_TABLE)

#define FIXMAP_PAGE_NORMAL  PAGE_KERNEL

#ifndef __ASSEMBLY__

#include <list.h>
#include <atomic.h>

extern unsigned long va_pa_offset;
extern struct page *mem_map;

extern unsigned long pfn_base;
#define ARCH_PFN_OFFSET (pfn_base)

typedef struct {
    unsigned long pgprot;
} pgprot_t;

typedef struct {
    unsigned long pgd;
} pgd_t;

typedef struct {
    unsigned long pmd;
} pmd_t;

typedef struct {
    unsigned long pte;
} pte_t;

typedef struct {
    u64 val;
} pfn_t;

typedef struct page *pgtable_t;

struct page {
    /* Atomic flags, some possibly updated asynchronously */
    unsigned long flags;

    struct {    /* Page cache and anonymous pages */
        struct list_head lru;

        /* See page-flags.h for PAGE_MAPPING_FLAGS */
        struct address_space *mapping;
        pgoff_t index;  /* Our offset within mapping. */

        /* Indicates order in the buddy system if PageBuddy */
        unsigned long private;
    };

    struct {    /* slab */
        struct list_head slab_list;
        struct kmem_cache *slab_cache; /* not slob */
        void *freelist; /* first free object */
        void *s_mem;    /* first object */
    };

    struct {    /* Tail pages of compound page */
        unsigned long compound_head;    /* Bit zero is set */
    };

    union {
        unsigned int page_type;
        unsigned int active;
    };

    /* Usage count. *DO NOT USE DIRECTLY*. See page_ref.h */
    atomic_t _refcount;
};

static inline unsigned long pmd_pfn(pmd_t pmd)
{
    return pmd_val(pmd) >> _PAGE_PFN_SHIFT;
}

#include <log2.h>
#define STRUCT_PAGE_MAX_SHIFT   (order_base_2(sizeof(struct page)))

#endif /* !__ASSEMBLY__ */

#ifndef pte_none
#define pte_none(pte) (!pte_val(pte))
#endif

#ifndef pmd_none
#define pmd_none(pmd) (!pmd_val(pmd))
#endif

#ifndef pgd_none
#define pgd_none(pgd) (!pgd_val(pgd))
#endif

/* test whether an address (unsigned long or pointer) is aligned to PAGE_SIZE */
#define PAGE_ALIGNED(addr)  IS_ALIGNED((unsigned long)(addr), PAGE_SIZE)
#define PAGE_ALIGN(addr)    _ALIGN(addr, PAGE_SIZE)

#define page_private(page)  ((page)->private)

#define virt_addr_valid(vaddr)  (pfn_valid(virt_to_pfn(vaddr)))

#define copy_user_page(vto, vfrom, vaddr, topg) \
    memcpy((vto), (vfrom), PAGE_SIZE)

#define clear_user_page(pgaddr, vaddr, page) \
    memset((pgaddr), 0, PAGE_SIZE)

#define clear_page(pgaddr) memset((pgaddr), 0, PAGE_SIZE)

#endif /* _PAGE_H */

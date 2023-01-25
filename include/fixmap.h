/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 Western Digital Corporation or its affiliates.
 */

#ifndef _ASM_RISCV_FIXMAP_H
#define _ASM_RISCV_FIXMAP_H

#include <page.h>
#include <sizes.h>

enum fixed_addresses {
    FIX_HOLE,
#define FIX_FDT_SIZE    SZ_1M
    FIX_FDT_END,
    FIX_FDT = FIX_FDT_END + FIX_FDT_SIZE / PAGE_SIZE - 1,
    FIX_PTE,
    FIX_PMD,
    FIX_TEXT_POKE1,
    FIX_TEXT_POKE0,
    FIX_EARLYCON_MEM_BASE,
    __end_of_fixed_addresses
};

#define fix_to_virt(x)  (FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define virt_to_fix(x)  ((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

#define FIXMAP_PAGE_CLEAR __pgprot(0)

#define clear_fixmap(idx) __set_fixmap(idx, 0, FIXMAP_PAGE_CLEAR)

#define __set_fixmap_offset(idx, phys, flags)               \
({                                  \
    unsigned long ________addr;                 \
    __set_fixmap(idx, phys, flags);                 \
    ________addr = fix_to_virt(idx) + ((phys) & (PAGE_SIZE - 1));   \
    ________addr;                           \
})

#define set_fixmap_offset(idx, phys) \
    __set_fixmap_offset(idx, phys, FIXMAP_PAGE_NORMAL)

void __set_fixmap(enum fixed_addresses idx, phys_addr_t phys, pgprot_t prot);

#endif /* _ASM_RISCV_FIXMAP_H */

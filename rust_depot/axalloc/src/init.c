// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>
#include <rust_alloc.h>

extern void *__rg_alloc;
extern void *__rg_alloc_zeroed;
extern void *__rg_dealloc;
extern void *__rg_realloc;

int
init_module(void)
{
    sbi_puts("module[axalloc]: init begin ...\n");
    rg_alloc = (rg_alloc_t) &__rg_alloc;
    rg_alloc_zeroed = (rg_alloc_zeroed_t) &__rg_alloc_zeroed;
    rg_dealloc = (rg_dealloc_t) &__rg_dealloc;
    rg_realloc = (rg_realloc_t) &__rg_realloc;
    sbi_puts("module[axalloc]: init end!\n");
    return 0;
}

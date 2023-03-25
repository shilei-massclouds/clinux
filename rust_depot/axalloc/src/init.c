// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>
#include <rust_alloc.h>

extern rg_alloc_t __rg_alloc;
extern rg_alloc_t rg_alloc;

int
init_module(void)
{
    sbi_puts("module[axalloc]: init begin ...\n");
    rg_alloc = __rg_alloc;
    sbi_puts("module[axalloc]: init end!\n");
    return 0;
}

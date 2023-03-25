// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

int
init_module(void)
{
    sbi_puts("module[allocator]: init begin ...\n");
    sbi_puts("module[allocator]: init end!\n");
    return 0;
}

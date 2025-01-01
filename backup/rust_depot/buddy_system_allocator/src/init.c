// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

int
init_module(void)
{
    sbi_puts("module[buddy_system_allocator]: init begin ...\n");
    sbi_puts("module[buddy_system_allocator]: init end!\n");
    return 0;
}

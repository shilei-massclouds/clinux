// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void platform_init();

int
init_module(void)
{
    sbi_puts("module[axhal]: init begin ...\n");
    sbi_puts("platform_init...\n");
    platform_init();
    sbi_puts("module[axhal]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_memory();

int
init_module(void)
{
    sbi_puts("module[memtest]: init begin ...\n");
    test_memory();
    sbi_puts("module[memtest]: init end!\n");
    return 0;
}

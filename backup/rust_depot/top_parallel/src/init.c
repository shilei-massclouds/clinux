// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_parallel();

int
init_module(void)
{
    sbi_puts("module[parallel]: init begin ...\n");
    test_parallel();
    sbi_puts("module[parallel]: init end!\n");
    return 0;
}

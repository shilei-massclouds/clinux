// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_sleep();

int
init_module(void)
{
    sbi_puts("module[sleep]: init begin ...\n");
    test_sleep();
    sbi_puts("module[sleep]: init end!\n");
    return 0;
}

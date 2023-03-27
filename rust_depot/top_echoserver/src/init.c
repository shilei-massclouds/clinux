// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_echoserver();

int
init_module(void)
{
    sbi_puts("module[echoserver]: init begin ...\n");
    test_echoserver();
    sbi_puts("module[echoserver]: init end!\n");
    return 0;
}

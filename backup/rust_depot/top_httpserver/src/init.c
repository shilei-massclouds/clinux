// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_httpserver();

int
init_module(void)
{
    sbi_puts("module[httpserver]: init begin ...\n");
    test_httpserver();
    sbi_puts("module[httpserver]: init end!\n");
    return 0;
}

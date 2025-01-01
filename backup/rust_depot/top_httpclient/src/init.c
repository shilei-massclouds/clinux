// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void test_httpclient();

int
init_module(void)
{
    sbi_puts("module[httpclient]: init begin ...\n");
    test_httpclient();
    sbi_puts("module[httpclient]: init end!\n");
    return 0;
}

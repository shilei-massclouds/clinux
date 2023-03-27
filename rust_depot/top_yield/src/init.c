// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern bool libax_ready(void);

extern void test_yield();

int
init_module(void)
{
    sbi_puts("module[yield]: init begin ...\n");
    SBI_ASSERT(libax_ready());
    test_yield();
    sbi_puts("module[yield]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern bool libax_ready(void);

extern void test_exception();

int
init_module(void)
{
    sbi_puts("module[test_exception]: init begin ...\n");
    SBI_ASSERT(libax_ready());
    test_exception();
    sbi_puts("module[test_exception]: init end!\n");
    return 0;
}

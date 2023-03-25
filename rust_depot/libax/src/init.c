// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern bool axruntime_ready(void);

int
init_module(void)
{
    sbi_puts("module[libax]: init begin ...\n");
    SBI_ASSERT(axruntime_ready());
    sbi_puts("module[libax]: init end!\n");
    return 0;
}
// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

int
init_module(void)
{
    sbi_puts("module[axerror]: init begin ...\n");
    sbi_puts("module[axerror]: init end!\n");
    return 0;
}

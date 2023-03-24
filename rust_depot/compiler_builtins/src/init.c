// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

int
init_module(void)
{
    sbi_puts("module[compiler_builtins]: init begin ...\n");
    sbi_puts("module[compiler_builtins]: init end!\n");
    return 0;
}

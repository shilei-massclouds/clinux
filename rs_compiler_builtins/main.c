// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[rs_compiler_builtins]: init begin ...\n");
    printk("module[rs_compiler_builtins]: init end!\n");
    return 0;
}

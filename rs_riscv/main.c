// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[riscv]: init begin ...\n");
    printk("module[riscv]: init end!\n");
    return 0;
}

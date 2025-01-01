// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[kalloc]: init begin ...\n");
    printk("module[kalloc]: init end!\n");
    return 0;
}

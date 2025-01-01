// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[lib]: init begin ...\n");
    printk("module[lib]: init end!\n");
    return 0;
}

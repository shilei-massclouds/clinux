// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>

int
init_module(void)
{
    printk("module[printk]: init begin ...\n");
    printk("module[printk]: init end!\n");
    return 0;
}

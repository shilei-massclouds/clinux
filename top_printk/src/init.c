// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>

int
init_module(void)
{
    printk("module[top_printk]: init begin ...\n");
    printk("module[top_printk]: init end!\n");
    return 0;
}

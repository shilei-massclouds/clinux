// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axalloc]: init begin ...\n");
    printk("module[axalloc]: init end!\n");
    return 0;
}

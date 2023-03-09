// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axhal]: init begin ...\n");
    printk("module[axhal]: init end!\n");
    return 0;
}

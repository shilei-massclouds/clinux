// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[libax]: init begin ...\n");
    printk("module[libax]: init end!\n");
    return 0;
}

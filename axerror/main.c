// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axerror]: init begin ...\n");
    printk("module[axerror]: init end!\n");
    return 0;
}

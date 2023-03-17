// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[driver_net]: init begin ...\n");
    printk("module[driver_net]: init end!\n");
    return 0;
}

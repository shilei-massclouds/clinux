// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axlog]: init begin ...\n");
    printk("module[axlog]: init end!\n");
    return 0;
}

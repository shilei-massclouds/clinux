// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axtask]: init begin ...\n");
    printk("module[axtask]: init end!\n");
    return 0;
}

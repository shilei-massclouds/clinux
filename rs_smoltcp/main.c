// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[smoltcp]: init begin ...\n");
    printk("module[smoltcp]: init end!\n");
    return 0;
}

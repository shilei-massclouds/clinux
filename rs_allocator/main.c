// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[rs_allocator]: init begin ...\n");
    printk("module[rs_allocator]: init end!\n");
    return 0;
}

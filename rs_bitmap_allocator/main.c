// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[bitmap_allocator]: init begin ...\n");
    printk("module[bitmap_allocator]: init end!\n");
    return 0;
}

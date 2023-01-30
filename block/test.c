// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

static int
init_module(void)
{
    printk("module[test_block]: init begin ...\n");
    printk("module[test_block]: init end!\n");
}

// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

static int
init_module(void)
{
    printk("module[test_scatterlist]: init begin ...\n");
    printk("module[test_scatterlist]: init end!\n");
    return 0;
}

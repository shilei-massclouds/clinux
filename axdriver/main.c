// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[axdriver]: init begin ...\n");
    printk("module[axdriver]: init end!\n");
    return 0;
}

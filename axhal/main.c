// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool memblock_ready(void);

int
init_module(void)
{
    printk("module[axhal]: init begin ...\n");
    BUG_ON(!memblock_ready());
    printk("module[axhal]: init end!\n");
    return 0;
}

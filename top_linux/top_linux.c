// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool userboot_ready;

int
init_module(void)
{
    printk("module[top_linux]: init begin ...\n");

    BUG_ON(!userboot_ready);

    printk("module[top_linux]: init end!\n");

    return 0;
}

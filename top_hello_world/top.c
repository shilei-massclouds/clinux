// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool ready();

int
init_module(void)
{
    printk("module[top_hello_world]: init begin ...\n");
    BUG_ON(!ready());
    //printk("[Hello, World!]\n");
    printk("module[top_hello_world]: init end!\n");
    return 0;
}

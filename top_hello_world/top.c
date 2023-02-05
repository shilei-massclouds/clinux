// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

static int
init_module(void)
{
    printk("module[top_hello_world]: init begin ...\n");
    printk("[Hello, World!]\n");
    printk("module[top_hello_world]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>

int
init_module(void)
{
    printk("module[helloworld]: init begin ...\n");
    printk("C: Hello world!\n");
    printk("module[helloworld]: init end!\n");
    return 0;
}

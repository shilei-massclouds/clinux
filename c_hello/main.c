// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <export.h>

bool
ready(void)
{
    return true;
}
EXPORT_SYMBOL(ready);

int
init_module(void)
{
    printk("module[c_hello]: init begin ...\n");
    printk("C: Hello world!\n");
    printk("module[c_hello]: init end!\n");
    return 0;
}

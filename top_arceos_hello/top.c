// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern void say_hello(void);

int
init_module(void)
{
    printk("module[top_arceos_hello]: init begin ...\n");
    say_hello();
    printk("module[top_arceos_hello]: init end!\n");
    return 0;
}

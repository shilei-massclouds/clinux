// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern void test_memory(void);

int
init_module(void)
{
    printk("module[top_arceos_memtest]: init begin ...\n");
    test_memory();
    printk("module[top_arceos_memtest]: init end!\n");
    return 0;
}

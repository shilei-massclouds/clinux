// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool rs_memory_addr_ready(void);

int
init_module(void)
{
    printk("module[top_memory_addr]: init begin ...\n");
    BUG_ON(!rs_memory_addr_ready());
    printk("module[top_memory_addr]: init end!\n");
    return 0;
}

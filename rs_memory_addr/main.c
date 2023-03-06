// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <export.h>

bool
rs_memory_addr_ready(void)
{
    return true;
}
EXPORT_SYMBOL(rs_memory_addr_ready);

int
init_module(void)
{
    printk("module[rs_memory_addr]: init begin ...\n");
    printk("module[rs_memory_addr]: init end!\n");
    return 0;
}

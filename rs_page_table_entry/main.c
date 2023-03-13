// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <export.h>

int
init_module(void)
{
    printk("module[rs_page_table_entry]: init begin ...\n");
    printk("module[rs_page_table_entry]: init end!\n");
    return 0;
}

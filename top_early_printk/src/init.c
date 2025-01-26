// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>

int
init_module(void)
{
    sbi_puts("module[top_early_printk]: init begin ...\n");
    early_printk("module[top_early_printk]: init begin ...\n");
    early_printk("module[top_early_printk]: init end!\n");
    sbi_puts("module[top_early_printk]: init end!\n");
    return 0;
}

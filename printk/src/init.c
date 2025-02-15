// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/irq_work.h>
#include <linux/wait.h>
#include "../../booter/src/booter.h"

int
cl_printk_init(void)
{
    sbi_puts("module[printk]: init begin ...\n");
    sbi_puts("module[printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_printk_init);

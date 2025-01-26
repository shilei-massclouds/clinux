// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/module.h>
#include "../../booter/src/booter.h"

static int __init init_early_printk(void)
{
    sbi_puts("module[early_printk]: init begin ...\n");
    sbi_puts("module[early_printk]: init end!\n");
    return 0;
}
module_init(init_early_printk);

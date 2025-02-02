// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include "../../booter/src/booter.h"

int
cl_top_spinlock_init(void)
{
    sbi_puts("module[top_spinlock]: init begin ...\n");
    sbi_puts("module[top_spinlock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_spinlock_init);

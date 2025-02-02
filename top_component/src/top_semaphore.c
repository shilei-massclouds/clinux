// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include "../../booter/src/booter.h"

int
cl_top_semaphore_init(void)
{
    sbi_puts("module[top_semaphore]: init begin ...\n");
    sbi_puts("module[top_semaphore]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_semaphore_init);

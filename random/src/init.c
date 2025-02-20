// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_random_init(void)
{
    sbi_puts("module[random]: init begin ...\n");
    sbi_puts("module[random]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_random_init);

int proc_dostring(struct ctl_table *table, int write,
          void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl in 'lib'.");
}

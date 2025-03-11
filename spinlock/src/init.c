// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_spinlock_init(void)
{
    sbi_puts("module[spinlock]: init begin ...\n");
    sbi_puts("module[spinlock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_spinlock_init);

__weak struct task_struct *get_current(void)
{
    booter_panic("No current!\n");
}
EXPORT_SYMBOL(get_current);

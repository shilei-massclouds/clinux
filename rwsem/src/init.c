// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/debug.h>
#include <linux/sched/wake_q.h>
#include "../../booter/src/booter.h"

int
cl_rwsem_init(void)
{
    sbi_puts("module[rwsem]: init begin ...\n");
    sbi_puts("module[rwsem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_rwsem_init);

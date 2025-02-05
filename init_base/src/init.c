// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include "../../booter/src/booter.h"

int
cl_init_base_init(void)
{
    sbi_puts("module[init_base]: init begin ...\n");
    riscv_current_is_tp = &init_task;
    sbi_puts("module[init_base]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_init_base_init);

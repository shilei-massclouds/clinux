// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_semaphore_init(void)
{
    sbi_puts("module[semaphore]: init begin ...\n");
    sbi_puts("module[semaphore]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_semaphore_init);

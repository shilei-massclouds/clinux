// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include "../../booter/src/booter.h"

int
cl_ipc_namespace_init(void)
{
    sbi_puts("module[ipc_namespace]: init begin ...\n");
    sbi_puts("module[ipc_namespace]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipc_namespace_init);

DEFINE_SPINLOCK(mq_lock);

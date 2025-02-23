// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_pid_namespace_init(void)
{
    sbi_puts("module[pid_namespace]: init begin ...\n");
    sbi_puts("module[pid_namespace]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_pid_namespace_init);

DEFINE_ENABLE_FUNC(pid_namespace);

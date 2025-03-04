// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_oom_kill_init(void)
{
    sbi_puts("module[oom_kill]: init begin ...\n");
    sbi_puts("module[oom_kill]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_oom_kill_init);

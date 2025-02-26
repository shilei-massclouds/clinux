// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_workingset_init(void)
{
    sbi_puts("module[workingset]: init begin ...\n");
    sbi_puts("module[workingset]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_workingset_init);

DEFINE_ENABLE_FUNC(workingset);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_groups_init(void)
{
    sbi_puts("module[groups]: init begin ...\n");
    sbi_puts("module[groups]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_groups_init);

DEFINE_ENABLE_FUNC(groups);

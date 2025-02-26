// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/mm_types.h>
#include "../../booter/src/booter.h"

int
cl_interval_tree_init(void)
{
    sbi_puts("module[interval_tree]: init begin ...\n");
    sbi_puts("module[interval_tree]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_interval_tree_init);

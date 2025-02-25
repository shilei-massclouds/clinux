// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_fs_stat_init(void)
{
    sbi_puts("module[fs_stat]: init begin ...\n");
    sbi_puts("module[fs_stat]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_fs_stat_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/path.h>
#include "../../booter/src/booter.h"

int
cl_fs_struct_init(void)
{
    sbi_puts("module[fs_struct]: init begin ...\n");
    sbi_puts("module[fs_struct]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_fs_struct_init);

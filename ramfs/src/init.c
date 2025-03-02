// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/fs_parser.h>
#include "../../booter/src/booter.h"

int
cl_ramfs_init(void)
{
    sbi_puts("module[ramfs]: init begin ...\n");
    sbi_puts("module[ramfs]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ramfs_init);

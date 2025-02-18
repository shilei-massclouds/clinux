// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_top_inode_init(void)
{
    sbi_puts("module[top_inode]: init begin ...\n");
    inode_init();
    sbi_puts("module[top_inode]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_inode_init);

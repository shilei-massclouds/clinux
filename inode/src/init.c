// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_inode_init(void)
{
    sbi_puts("module[inode]: init begin ...\n");
    sbi_puts("module[inode]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_inode_init);

const struct file_operations pipefifo_fops;
int sysctl_vfs_cache_pressure __read_mostly = 100;
const struct file_operations def_blk_fops;

int __mnt_want_write_file(struct file *file)
{
    booter_panic("No impl!\n");
}

int remove_inode_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_block_dev_init(void)
{
    sbi_puts("module[block_dev]: init begin ...\n");
    sbi_puts("module[block_dev]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_block_dev_init);

/*
void clear_inode(struct inode *inode)
{
    booter_panic("No impl!\n");
}
*/
int simple_statfs(struct dentry *dentry, struct kstatfs *buf)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_statfs);

void invalidate_inode_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}
struct vfsmount *kern_mount(struct file_system_type *type)
{
    booter_panic("No impl!\n");
}
struct pseudo_fs_context *init_pseudo(struct fs_context *fc,
                    unsigned long magic)
{
    booter_panic("No impl!\n");
}

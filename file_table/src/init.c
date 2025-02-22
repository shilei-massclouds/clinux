// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_file_table_init(void)
{
    sbi_puts("module[file_table]: init begin ...\n");
    sbi_puts("module[file_table]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_file_table_init);

int __fsnotify_parent(struct dentry *dentry, __u32 mask, const void *data,
              int data_type)
{
    booter_panic("No impl in 'workqueue'.");
}
void locks_remove_file(struct file *filp)
{
    booter_panic("No impl in 'workqueue'.");
}
void dissolve_on_fput(struct vfsmount *mnt)
{
    booter_panic("No impl in 'workqueue'.");
}
void eventpoll_release_file(struct file *file)
{
    booter_panic("No impl in 'workqueue'.");
}


// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/proc_ns.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_fs_namespace_init(void)
{
    sbi_puts("module[fs_namespace]: init begin ...\n");
    sbi_puts("module[fs_namespace]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_fs_namespace_init);

DEFINE_ENABLE_FUNC(fs_namespace);

/*
void set_fs_root(struct fs_struct *fs, const struct path *path)
{
    booter_panic("No impl!\n");
}
void set_fs_pwd(struct fs_struct *fs, const struct path *path)
{
    booter_panic("No impl!\n");
}
*/

__weak int __init shmem_init(void)
{
    booter_panic("No impl!\n");
}

const struct proc_ns_operations mntns_operations;

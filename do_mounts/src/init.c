// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_do_mounts_init(void)
{
    sbi_puts("module[do_mounts]: init begin ...\n");
    sbi_puts("module[do_mounts]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_do_mounts_init);

int __init nfs_root_data(char **root_device, char **root_data)
{
    booter_panic("No impl.\n");
}

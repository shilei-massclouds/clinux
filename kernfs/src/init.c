// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/fs_parser.h>
#include <linux/xattr.h>
#include <linux/seq_file.h>
#include "../../booter/src/booter.h"

int
cl_kernfs_init(void)
{
    sbi_puts("module[kernfs]: init begin ...\n");
    sbi_puts("module[kernfs]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_kernfs_init);

/*
loff_t generic_file_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_file_llseek);
*/

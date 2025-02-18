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

int get_tree_nodev(struct fs_context *fc,
          int (*fill_super)(struct super_block *sb,
                    struct fs_context *fc))
{
    booter_panic("No impl!\n");
}
int fs_param_is_u32(struct p_log *log, const struct fs_parameter_spec *p,
            struct fs_parameter *param, struct fs_parse_result *result)
{
    booter_panic("No impl!\n");
}

int __fs_parse(struct p_log *log,
         const struct fs_parameter_spec *desc,
         struct fs_parameter *param,
         struct fs_parse_result *result)
{
    booter_panic("No impl!\n");
}

int __set_page_dirty_no_writeback(struct page *page)
{
    booter_panic("No impl!\n");
}



const struct file_operations simple_dir_operations;
const struct inode_operations page_symlink_inode_operations;
const struct inode_operations ramfs_file_inode_operations;
const struct file_operations ramfs_file_operations;

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

struct dentry *d_make_root(struct inode *root_inode)
{
    booter_panic("No impl!\n");
}
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

void inode_init_owner(struct inode *inode, const struct inode *dir,
            umode_t mode)
{
    booter_panic("No impl!\n");
}
struct inode *new_inode(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
void init_special_inode(struct inode *inode, umode_t mode, dev_t rdev)
{
    booter_panic("No impl!\n");
}
struct timespec64 current_time(struct inode *inode)
{
    booter_panic("No impl!\n");
}
unsigned int get_next_ino(void)
{
    booter_panic("No impl!\n");
}
void inode_nohighmem(struct inode *inode)
{
    booter_panic("No impl!\n");
}
int simple_readpage(struct file *file, struct page *page)
{
    booter_panic("No impl!\n");
}
void inc_nlink(struct inode *inode)
{
    booter_panic("No impl!\n");
}
int __set_page_dirty_no_writeback(struct page *page)
{
    booter_panic("No impl!\n");
}
int simple_write_end(struct file *file, struct address_space *mapping,
            loff_t pos, unsigned len, unsigned copied,
            struct page *page, void *fsdata)
{
    booter_panic("No impl!\n");
}

int simple_write_begin(struct file *file, struct address_space *mapping,
            loff_t pos, unsigned len, unsigned flags,
            struct page **pagep, void **fsdata)
{
    booter_panic("No impl!\n");
}


const struct file_operations simple_dir_operations;
const struct inode_operations page_symlink_inode_operations;
const struct inode_operations ramfs_file_inode_operations;
const struct file_operations ramfs_file_operations;

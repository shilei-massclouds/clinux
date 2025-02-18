// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
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

void generic_fillattr(struct inode *inode, struct kstat *stat)
{
    booter_panic("No impl!\n");
}
int setattr_prepare(struct dentry *dentry, struct iattr *attr)
{
    booter_panic("No impl!\n");
}
int noop_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
    booter_panic("No impl!\n");
}
int seq_release(struct inode *inode, struct file *file)
{
    booter_panic("No impl!\n");
}
void make_empty_dir_inode(struct inode *inode)
{
    booter_panic("No impl!\n");
}
/*
int fsnotify(__u32 mask, const void *data, int data_type, struct inode *dir,
         const struct qstr *file_name, struct inode *inode, u32 cookie)
{
    booter_panic("No impl!\n");
}
*/
struct super_block *sget_fc(struct fs_context *fc,
                int (*test)(struct super_block *, struct fs_context *),
                int (*set)(struct super_block *, struct fs_context *))
{
    booter_panic("No impl!\n");
}
struct dentry *d_splice_alias(struct inode *inode, struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
void unmap_mapping_range(struct address_space *mapping,
        loff_t const holebegin, loff_t const holelen, int even_cows)
{
    booter_panic("No impl!\n");
}
int generic_permission(struct inode *inode, int mask)
{
    booter_panic("No impl!\n");
}
int simple_xattr_set(struct simple_xattrs *xattrs, const char *name,
             const void *value, size_t size, int flags,
             ssize_t *removed_size)
{
    booter_panic("No impl!\n");
}
/*
void truncate_inode_pages_final(struct address_space *mapping)
{
    booter_panic("No impl!\n");
}
*/
ssize_t generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
int set_anon_super_fc(struct super_block *sb, struct fs_context *fc)
{
    booter_panic("No impl!\n");
}
void lockref_get(struct lockref *lockref)
{
    booter_panic("No impl!\n");
}
void deactivate_locked_super(struct super_block *s)
{
    booter_panic("No impl!\n");
}
void kfree_link(void *p)
{
    booter_panic("No impl!\n");
}
int seq_open(struct file *file, const struct seq_operations *op)
{
    booter_panic("No impl!\n");
}
loff_t generic_file_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl!\n");
}
ssize_t simple_xattr_list(struct inode *inode, struct simple_xattrs *xattrs,
              char *buffer, size_t size)
{
    booter_panic("No impl!\n");
}

const char *xattr_full_name(const struct xattr_handler *handler,
                const char *name)
{
    booter_panic("No impl!\n");
}
void setattr_copy(struct inode *inode, const struct iattr *attr)
{
    booter_panic("No impl!\n");
}
struct dentry *d_obtain_alias(struct inode *inode)
{
    booter_panic("No impl!\n");
}
int simple_xattr_get(struct simple_xattrs *xattrs, const char *name,
             void *buffer, size_t size)
{
    booter_panic("No impl!\n");
}

int seq_dentry(struct seq_file *m, struct dentry *dentry, const char *esc)
{
    booter_panic("No impl!\n");
}

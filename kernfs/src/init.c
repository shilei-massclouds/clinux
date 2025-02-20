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

void unmap_mapping_range(struct address_space *mapping,
        loff_t const holebegin, loff_t const holelen, int even_cows)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(unmap_mapping_range);

int simple_xattr_set(struct simple_xattrs *xattrs, const char *name,
             const void *value, size_t size, int flags,
             ssize_t *removed_size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_set);

loff_t generic_file_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_file_llseek);

ssize_t simple_xattr_list(struct inode *inode, struct simple_xattrs *xattrs,
              char *buffer, size_t size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_list);

const char *xattr_full_name(const struct xattr_handler *handler,
                const char *name)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(xattr_full_name);

int simple_xattr_get(struct simple_xattrs *xattrs, const char *name,
             void *buffer, size_t size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_get);

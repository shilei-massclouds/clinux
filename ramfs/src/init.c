// SPDX-License-Identifier: GPL-2.0+

#include <slab.h>
#include <stat.h>
#include <dcache.h>
#include <errno.h>
#include <ramfs.h>
#include <types.h>
#include <export.h>
#include <string.h>

#define RAMFS_DEFAULT_MODE  0755

struct ramfs_mount_opts {
    umode_t mode;
};

struct ramfs_fs_info {
    struct ramfs_mount_opts mount_opts;
};

static int
ramfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
    int error = -ENOSPC;
    struct inode *inode = ramfs_get_inode(dir->i_sb, dir, mode, dev);

    if (inode) {
        d_instantiate(dentry, inode);
        dget(dentry);   /* Extra count - pin the dentry in core */
        error = 0;
    }
    return error;
}

static int
ramfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    return ramfs_mknod(dir, dentry, mode | S_IFDIR, 0);
}

static const struct inode_operations
ramfs_dir_inode_operations = {
    .lookup = simple_lookup,
    .mkdir = ramfs_mkdir,
    .mknod = ramfs_mknod,
};

const struct file_operations def_blk_fops = {
};

extern const struct file_operations def_chr_fops;

void
init_special_inode(struct inode *inode, umode_t mode, dev_t rdev)
{
    inode->i_mode = mode;
    if (S_ISCHR(mode)) {
        inode->i_fop = &def_chr_fops;
        inode->i_rdev = rdev;
    } else if (S_ISBLK(mode)) {
        inode->i_fop = &def_blk_fops;
        inode->i_rdev = rdev;
    } else if (S_ISFIFO(mode))
        panic("no fifo dev!");
        //inode->i_fop = &pipefifo_fops;
    else if (S_ISSOCK(mode))
        ;   /* leave it no_open_fops */
    else
        panic("init_special_inode: bogus i_mode (%x) for inode %lu",
              mode, inode->i_ino);
}
EXPORT_SYMBOL(init_special_inode);

void
inode_init_owner(struct inode *inode, const struct inode *dir, umode_t mode)
{
    inode->i_mode = mode;
}
EXPORT_SYMBOL(inode_init_owner);

struct inode *
ramfs_get_inode(struct super_block *sb, const struct inode *dir,
                umode_t mode, dev_t dev)
{
    struct inode *inode = new_inode(sb);
    if (inode) {
        inode_init_owner(inode, dir, mode);
        switch (mode & S_IFMT) {
        default:
            init_special_inode(inode, mode, dev);
            break;
        case S_IFDIR:
            inode->i_op = &ramfs_dir_inode_operations;
            break;
        }
    }
    return inode;
}

static const struct super_operations ramfs_ops = {
};

static int
ramfs_fill_super(struct super_block *sb, struct fs_context *fc)
{
    struct inode *inode;
    struct ramfs_fs_info *fsi = sb->s_fs_info;

    sb->s_op = &ramfs_ops;

    inode = ramfs_get_inode(sb, NULL, S_IFDIR | fsi->mount_opts.mode, 0);
    sb->s_root = d_make_root(inode);
    if (!sb->s_root)
        return -ENOMEM;

    return 0;
}

static int
ramfs_get_tree(struct fs_context *fc)
{
    return get_tree_nodev(fc, ramfs_fill_super);
}

static const struct fs_context_operations ramfs_context_ops = {
    .get_tree = ramfs_get_tree,
};

int
ramfs_init_fs_context(struct fs_context *fc)
{
    struct ramfs_fs_info *fsi;

    fsi = kzalloc(sizeof(*fsi), GFP_KERNEL);
    if (!fsi)
        return -ENOMEM;

    fsi->mount_opts.mode = RAMFS_DEFAULT_MODE;
    fc->s_fs_info = fsi;
    fc->ops = &ramfs_context_ops;
    return 0;
}
EXPORT_SYMBOL(ramfs_init_fs_context);

static struct file_system_type ramfs_fs_type = {
    .name = "ramfs",
    .init_fs_context = ramfs_init_fs_context,
    .fs_flags = FS_USERNS_MOUNT,
};

int
init_module(void)
{
    printk("module[ramfs]: init begin ...\n");

    register_filesystem(&ramfs_fs_type);

    printk("module[ramfs]: init end!\n");
    return 0;
}

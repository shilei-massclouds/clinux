// SPDX-License-Identifier: GPL-2.0

#include <fs.h>
#include <slab.h>
#include <errno.h>
#include <blkdev.h>
#include <dcache.h>
#include <export.h>
#include <backing-dev-defs.h>

static LIST_HEAD(super_blocks);

static u32 unnamed_dev_ida;

static int
set_bdev_super(struct super_block *s, void *data)
{
    s->s_bdev = data;
    s->s_dev = s->s_bdev->bd_dev;
    s->s_bdi = s->s_bdev->bd_bdi;

    return 0;
}

static int
test_bdev_super(struct super_block *s, void *data)
{
    return (void *)s->s_bdev == data;
}

static struct super_block *
alloc_super(struct file_system_type *type, int flags)
{
    struct super_block *s;
    s = kzalloc(sizeof(struct super_block), GFP_USER);
    if (!s)
        return NULL;

    s->s_bdi = &noop_backing_dev_info;
    INIT_LIST_HEAD(&s->s_inodes);
    return s;
}

struct super_block *
sget(struct file_system_type *type,
     int (*test)(struct super_block *,void *),
     int (*set)(struct super_block *,void *),
     int flags,
     void *data)
{
    int err;
    struct super_block *old;
    struct super_block *s = NULL;

    if (test) {
        hlist_for_each_entry(old, &type->fs_supers, s_instances) {
            if (!test(old, data))
                continue;
            panic("already exist!");
        }
    }

    s = alloc_super(type, (flags & ~SB_SUBMOUNT));
    if (!s)
        panic("bad alloc!");

    err = set(s, data);
    if (err)
        return ERR_PTR(err);

    s->s_type = type;
    strlcpy(s->s_id, type->name, sizeof(s->s_id));
    list_add_tail(&s->s_list, &super_blocks);
    hlist_add_head(&s->s_instances, &type->fs_supers);
    get_filesystem(type);
    return s;
}

struct super_block *
sget_fc(struct fs_context *fc,
        int (*set)(struct super_block *, struct fs_context *))
{
    struct super_block *s = NULL;

    s = alloc_super(fc->fs_type, fc->sb_flags);
    if (!s)
        return ERR_PTR(-ENOMEM);

    s->s_fs_info = fc->s_fs_info;
    if (set(s, fc))
        panic("cannot set!");

    fc->s_fs_info = NULL;
    s->s_type = fc->fs_type;
    return s;
}

int
get_anon_bdev(dev_t *p)
{
    *p = MKDEV(0, unnamed_dev_ida++);
    return 0;
}
EXPORT_SYMBOL(get_anon_bdev);

int
set_anon_super(struct super_block *s, void *data)
{
    return get_anon_bdev(&s->s_dev);
}
EXPORT_SYMBOL(set_anon_super);

int
set_anon_super_fc(struct super_block *sb, struct fs_context *fc)
{
    return set_anon_super(sb, NULL);
}
EXPORT_SYMBOL(set_anon_super_fc);

int
vfs_get_super(struct fs_context *fc,
              int (*fill_super)(struct super_block *sb,
                                struct fs_context *fc))
{
    struct super_block *sb;
    sb = sget_fc(fc, set_anon_super_fc);
    if (IS_ERR(sb))
        return PTR_ERR(sb);

    BUG_ON(sb->s_root);
    if (fill_super(sb, fc))
        panic("cannot fill super!");

    sb->s_flags |= SB_ACTIVE;
    fc->root = dget(sb->s_root);
    return 0;
}

int
get_tree_nodev(struct fs_context *fc,
               int (*fill_super)(struct super_block *sb,
                                 struct fs_context *fc))
{
    return vfs_get_super(fc, fill_super);
}
EXPORT_SYMBOL(get_tree_nodev);

struct dentry *
mount_bdev(struct file_system_type *fs_type,
           int flags, const char *dev_name,
           int (*fill_super)(struct super_block *, void *, int))
{
    int error = 0;
    struct super_block *s;
    struct block_device *bdev;
    fmode_t mode = FMODE_READ | FMODE_EXCL;

    if (!(flags & SB_RDONLY))
        mode |= FMODE_WRITE;

    printk("%s: %s %x\n", __func__, dev_name, flags);
    bdev = blkdev_get_by_path(dev_name, mode, fs_type);
    if (IS_ERR(bdev))
        return ERR_CAST(bdev);

    s = sget(fs_type, test_bdev_super, set_bdev_super, flags|SB_NOSEC, bdev);
    if (IS_ERR(s))
        panic("bad super!");

    s->s_mode = mode;
    snprintf(s->s_id, sizeof(s->s_id), "%pg", bdev);
    sb_set_blocksize(s, block_size(bdev));

    error = fill_super(s, NULL, flags & SB_SILENT ? 1 : 0);
    if (error)
        panic("can not fill_super!");

    s->s_flags |= SB_ACTIVE;
    bdev->bd_super = s;
    return dget(s->s_root);
}
EXPORT_SYMBOL(mount_bdev);

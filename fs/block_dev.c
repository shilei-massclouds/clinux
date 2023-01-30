// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <bug.h>
#include <slab.h>
#include <stat.h>
#include <errno.h>
#include <genhd.h>
#include <mount.h>
#include <namei.h>
#include <blkdev.h>
#include <export.h>
#include <kdev_t.h>
#include <kernel.h>
#include <printk.h>
#include <string.h>
#include <backing-dev.h>

#define BDEVFS_MAGIC 0x62646576

struct bdev_inode {
    struct block_device bdev;
    struct inode vfs_inode;
};

static struct kmem_cache *bdev_cachep;

extern struct super_block *blockdev_superblock;

static inline unsigned long
hash(dev_t dev)
{
    return MAJOR(dev) + MINOR(dev);
}

static inline struct bdev_inode *
BDEV_I(struct inode *inode)
{
    return container_of(inode, struct bdev_inode, vfs_inode);
}

static int
bdev_test(struct inode *inode, void *data)
{
    return BDEV_I(inode)->bdev.bd_dev == *(dev_t *)data;
}

static int
bdev_set(struct inode *inode, void *data)
{
    BDEV_I(inode)->bdev.bd_dev = *(dev_t *)data;
    return 0;
}

void
unlock_new_inode(struct inode *inode)
{
    BUG_ON(!(inode->i_state & I_NEW));
    inode->i_state &= ~I_NEW & ~I_CREATING;
}
EXPORT_SYMBOL(unlock_new_inode);

struct block_device *
bdget(dev_t dev)
{
    struct block_device *bdev;
    struct inode *inode;

    inode = iget5_locked(blockdev_superblock, hash(dev),
                         bdev_test, bdev_set, &dev);
    if (!inode)
        return NULL;

    bdev = &BDEV_I(inode)->bdev;
    if (inode->i_state & I_NEW) {
        bdev->bd_super = NULL;
        bdev->bd_inode = inode;
        inode->i_mode = S_IFBLK;
        inode->i_rdev = dev;
        inode->i_bdev = bdev;
        unlock_new_inode(inode);
    }
    return bdev;
}

static struct block_device *
bd_acquire(struct inode *inode)
{
    struct block_device *bdev;

    bdev = inode->i_bdev;
    if (bdev)
        return bdev;

    printk("%s: i_rdev(%x)\n", __func__, inode->i_rdev);
    bdev = bdget(inode->i_rdev);
    if (bdev) {
        if (!inode->i_bdev)
            inode->i_bdev = bdev;
    }
    return bdev;
}

struct block_device *
lookup_bdev(const char *pathname)
{
    int error;
    struct path path;
    struct inode *inode;
    struct block_device *bdev;

    if (!pathname || !*pathname)
        return ERR_PTR(-EINVAL);

    printk("%s: pathname(%s)\n", __func__, pathname);
    error = kern_path(pathname, LOOKUP_FOLLOW, &path);
    if (error)
        return ERR_PTR(error);

    inode = d_backing_inode(path.dentry);
    error = -ENOTBLK;
    if (!S_ISBLK(inode->i_mode))
        goto fail;
    error = -ENOMEM;
    bdev = bd_acquire(inode);
    if (!bdev) {
        panic("bad bdev!");
        goto fail;
    }
out:
    path_put(&path);
    return bdev;
fail:
    bdev = ERR_PTR(error);
    goto out;
}
EXPORT_SYMBOL(lookup_bdev);

static struct gendisk *
bdev_get_gendisk(struct block_device *bdev, int *partno)
{
    struct gendisk *disk = get_gendisk(bdev->bd_dev, partno);

    if (!disk)
        return NULL;
    /*
     * Now that we hold gendisk reference we make sure bdev we looked up is
     * not stale. If it is, it means device got removed and created before
     * we looked up gendisk and we fail open in such case. Associating
     * unhashed bdev with newly created gendisk could lead to two bdevs
     * (and thus two independent caches) being associated with one device
     * which is bad.
     */
    if (inode_unhashed(bdev->bd_inode)) {
        return NULL;
    }
    return disk;
}

int
bd_prepare_to_claim(struct block_device *bdev,
                    struct block_device *whole,
                    void *holder)
{
    /* if claiming is already in progress, wait for it to finish */
    if (whole->bd_claiming)
        panic("already claiming!");

    /* yay, all mine */
    whole->bd_claiming = holder;
    return 0;
}
EXPORT_SYMBOL(bd_prepare_to_claim); /* only for the loop driver */

void
bd_set_size(struct block_device *bdev, loff_t size)
{
    i_size_write(bdev->bd_inode, size);
}
EXPORT_SYMBOL(bd_set_size);

static void
set_init_blocksize(struct block_device *bdev)
{
    bdev->bd_inode->i_blkbits = blksize_bits(bdev_logical_block_size(bdev));
}

static void
bd_clear_claiming(struct block_device *whole, void *holder)
{
    whole->bd_claiming = NULL;
}

static void
bd_finish_claiming(struct block_device *bdev,
                   struct block_device *whole,
                   void *holder)
{
    bd_clear_claiming(whole, holder);
}

static int
__blkdev_get(struct block_device *bdev,
             fmode_t mode,
             void *holder,
             int for_part)
{
    int ret;
    int partno;
    struct gendisk *disk;

    BUG_ON(for_part);

    disk = bdev_get_gendisk(bdev, &partno);
    BUG_ON(partno);
    if (mode & FMODE_EXCL) {
        ret = bd_prepare_to_claim(bdev, bdev, holder);
        if (ret)
            panic("%s: can not claim!", __func__);
    }

    bdev->bd_disk = disk;
    bdev->bd_partno = partno;
    bdev->bd_part = disk_get_part(disk, partno);
    BUG_ON(!bdev->bd_part);

    ret = 0;
    if (disk->fops->open) {
        ret = disk->fops->open(bdev, mode);
    } else {
        panic("can not open disk!");
    }

    if (!ret) {
        bd_set_size(bdev, (loff_t)get_capacity(disk)<<9);
        set_init_blocksize(bdev);
    }

    if (bdev->bd_bdi == &noop_backing_dev_info) {
        bdev->bd_bdi = disk->queue->backing_dev_info;
    }

    bd_finish_claiming(bdev, bdev, holder);
    return 0;
}

int
blkdev_get(struct block_device *bdev, fmode_t mode, void *holder)
{
    return __blkdev_get(bdev, mode, holder, 0);
}
EXPORT_SYMBOL(blkdev_get);

struct block_device *
blkdev_get_by_path(const char *path, fmode_t mode, void *holder)
{
    int err;
    struct block_device *bdev;

    printk("%s: mode(%x) path(%s)\n", __func__, mode, path);
    bdev = lookup_bdev(path);
    if (IS_ERR(bdev))
        return bdev;

    err = blkdev_get(bdev, mode, holder);
    if (err)
        return ERR_PTR(err);

    return bdev;
}
EXPORT_SYMBOL(blkdev_get_by_path);

static struct inode *
bdev_alloc_inode(struct super_block *sb)
{
    struct bdev_inode *ei = kmem_cache_alloc(bdev_cachep, GFP_KERNEL);
    if (!ei)
        return NULL;
    return &ei->vfs_inode;
}

static const struct super_operations bdev_sops = {
    .alloc_inode = bdev_alloc_inode,
};

static int
bd_init_fs_context(struct fs_context *fc)
{
    struct pseudo_fs_context *ctx = init_pseudo(fc, BDEVFS_MAGIC);
    if (!ctx)
        return -ENOMEM;
    ctx->ops = &bdev_sops;
    return 0;
}

static void
init_once(void *foo)
{
    struct bdev_inode *ei = (struct bdev_inode *) foo;
    struct block_device *bdev = &ei->bdev;

    memset(bdev, 0, sizeof(*bdev));
    bdev->bd_bdi = &noop_backing_dev_info;
    inode_init_once(&ei->vfs_inode);
}

static struct file_system_type bd_type = {
    .name = "bdev",
    .init_fs_context = bd_init_fs_context,
};

void
bdev_cache_init(void)
{
    int err;
    static struct vfsmount *bd_mnt;

    bdev_cachep = kmem_cache_create("bdev_cache",
                                    sizeof(struct bdev_inode),
                                    0,
                                    (SLAB_HWCACHE_ALIGN|
                                     SLAB_RECLAIM_ACCOUNT|
                                     SLAB_MEM_SPREAD|
                                     SLAB_ACCOUNT|
                                     SLAB_PANIC),
                                    init_once);

    err = register_filesystem(&bd_type);
    if (err)
        panic("Cannot register bdev pseudo-fs");
    bd_mnt = kern_mount(&bd_type);
    if (IS_ERR(bd_mnt))
        panic("Cannot create bdev pseudo-fs");
    blockdev_superblock = bd_mnt->mnt_sb;   /* For writeback */
}

int
set_blocksize(struct block_device *bdev, int size)
{
    /* Size must be a power of two, and between 512 and PAGE_SIZE */
    if (size > PAGE_SIZE || size < 512 || !is_power_of_2(size))
        return -EINVAL;

    /* Size cannot be smaller than the size supported by the device */
    if (size < bdev_logical_block_size(bdev))
        return -EINVAL;

    /* Don't change the size if it is same as current */
    if (bdev->bd_inode->i_blkbits != blksize_bits(size))
        bdev->bd_inode->i_blkbits = blksize_bits(size);
    return 0;
}
EXPORT_SYMBOL(set_blocksize);

int
sb_set_blocksize(struct super_block *sb, int size)
{
    if (set_blocksize(sb->s_bdev, size))
        return 0;
    /* If we get here, we know size is power of two
     * and it's value is between 512 and PAGE_SIZE */
    sb->s_blocksize = size;
    sb->s_blocksize_bits = blksize_bits(size);
    return sb->s_blocksize;
}
EXPORT_SYMBOL(sb_set_blocksize);

int sb_min_blocksize(struct super_block *sb, int size)
{
    int minsize = bdev_logical_block_size(sb->s_bdev);
    if (size < minsize)
        size = minsize;
    return sb_set_blocksize(sb, size);
}
EXPORT_SYMBOL(sb_min_blocksize);

struct block_device *_I_BDEV(struct inode *inode)
{
    return &BDEV_I(inode)->bdev;
}

int bdev_read_page(struct block_device *bdev, sector_t sector,
                   struct page *page)
{
    const struct block_device_operations *ops = bdev->bd_disk->fops;

    if (!ops->rw_page)
        return -EOPNOTSUPP;

    return ops->rw_page(bdev, sector + get_start_sect(bdev),
                        page, REQ_OP_READ);
}
EXPORT_SYMBOL(bdev_read_page);

void block_dev_init(void)
{
    I_BDEV = _I_BDEV;
}

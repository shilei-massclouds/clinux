/* SPDX-License-Identifier: GPL-2.0-only */

#include <fs.h>
#include <slab.h>
#include <errno.h>
#include <mount.h>
#include <namei.h>
#include <export.h>
#include <kernel.h>
#include <limits.h>
#include <printk.h>
#include <string.h>
#include <current.h>
#include <uaccess.h>
#include <syscalls.h>
#include <hashtable.h>

static struct kmem_cache *mnt_cache;

static unsigned int m_hash_mask;
static unsigned int m_hash_shift;
static unsigned int mp_hash_mask;
static unsigned int mp_hash_shift;

static struct hlist_head *mount_hashtable;
static struct hlist_head *mountpoint_hashtable;

static inline struct hlist_head *
m_hash(struct vfsmount *mnt, struct dentry *dentry)
{
    unsigned long tmp = ((unsigned long)mnt / L1_CACHE_BYTES);
    tmp += ((unsigned long)dentry / L1_CACHE_BYTES);
    tmp = tmp + (tmp >> m_hash_shift);
    return &mount_hashtable[tmp & m_hash_mask];
}

static inline struct hlist_head *mp_hash(struct dentry *dentry)
{
    unsigned long tmp = ((unsigned long)dentry / L1_CACHE_BYTES);
    tmp = tmp + (tmp >> mp_hash_shift);
    return &mountpoint_hashtable[tmp & mp_hash_mask];
}

int
vfs_parse_fs_string(struct fs_context *fc,
                    const char *key,
                    const char *value,
                    size_t v_size)
{
    if (strcmp(key, "source") == 0) {
        if (fc->source)
            panic("VFS: Multiple sources");
        fc->source = kmemdup_nul(value, v_size, GFP_KERNEL);
        return 0;
    }

    panic("bad vfs params(%s)", key);
    return 0;
}
EXPORT_SYMBOL(vfs_parse_fs_string);

int
vfs_get_tree(struct fs_context *fc)
{
    int error;

    if (fc->root)
        return -EBUSY;

    error = fc->ops->get_tree(fc);
    if (error < 0)
        return error;

    return 0;
}
EXPORT_SYMBOL(vfs_get_tree);

struct mount *__lookup_mnt(struct vfsmount *mnt, struct dentry *dentry)
{
    struct mount *p;
    struct hlist_head *head = m_hash(mnt, dentry);

    hlist_for_each_entry(p, head, mnt_hash)
        if (&p->mnt_parent->mnt == mnt && p->mnt_mountpoint == dentry)
            return p;
    return NULL;
}

/*
 * lookup_mnt - Return the first child mount mounted at path
 *
 * "First" means first mounted chronologically.  If you create the
 * following mounts:
 *
 * mount /dev/sda1 /mnt
 * mount /dev/sda2 /mnt
 * mount /dev/sda3 /mnt
 *
 * Then lookup_mnt() on the base /mnt dentry in the root mount will
 * return successively the root dentry and vfsmount of /dev/sda1, then
 * /dev/sda2, then /dev/sda3, then NULL.
 *
 * lookup_mnt takes a reference to the found vfsmount.
 */
struct vfsmount *lookup_mnt(const struct path *path)
{
    struct mount *child_mnt;

    child_mnt = __lookup_mnt(path->mnt, path->dentry);
    return child_mnt ? &child_mnt->mnt : NULL;
}

static struct mountpoint *lookup_mountpoint(struct dentry *dentry)
{
    struct mountpoint *mp;
    struct hlist_head *chain = mp_hash(dentry);

    hlist_for_each_entry(mp, chain, m_hash) {
        if (mp->m_dentry == dentry) {
            mp->m_count++;
            return mp;
        }
    }
    return NULL;
}

static struct mountpoint *get_mountpoint(struct dentry *dentry)
{
    int ret;
    struct mountpoint *mp;
    struct mountpoint *new = NULL;

    if (d_mountpoint(dentry)) {
        /* might be worth a WARN_ON() */
        if (d_unlinked(dentry))
            return ERR_PTR(-ENOENT);

        mp = lookup_mountpoint(dentry);
        if (mp)
            return mp;
    }

    if (!new)
        new = kmalloc(sizeof(struct mountpoint), GFP_KERNEL);

    if (!new)
        return ERR_PTR(-ENOMEM);

    /* Exactly one processes may set d_mounted */
    ret = d_set_mounted(dentry);

    /* Add the new mountpoint to the hash table */
    new->m_dentry = dget(dentry);
    new->m_count = 1;
    hlist_add_head(&new->m_hash, mp_hash(dentry));
    INIT_HLIST_HEAD(&new->m_list);

    mp = new;
    new = NULL;
    return mp;
}

static struct mountpoint *lock_mount(struct path *path)
{
    struct vfsmount *mnt;
    struct dentry *dentry = path->dentry;

    mnt = lookup_mnt(path);
    if (likely(!mnt)) {
        struct mountpoint *mp = get_mountpoint(dentry);
        if (IS_ERR(mp))
            panic("bad mount point!");

        return mp;
    }

    panic("%s: !", __func__);
}

static inline void mnt_add_count(struct mount *mnt, int n)
{
    /* Todo */
}

void mnt_set_mountpoint(struct mount *mnt,
                        struct mountpoint *mp, struct mount *child_mnt)
{
    mp->m_count++;
    mnt_add_count(mnt, 1);  /* essentially, that's mntget */
    child_mnt->mnt_mountpoint = mp->m_dentry;
    child_mnt->mnt_parent = mnt;
    child_mnt->mnt_mp = mp;
    hlist_add_head(&child_mnt->mnt_mp_list, &mp->m_list);
}

static void __attach_mnt(struct mount *mnt, struct mount *parent)
{
    hlist_add_head(&mnt->mnt_hash, m_hash(&parent->mnt, mnt->mnt_mountpoint));
    list_add_tail(&mnt->mnt_child, &parent->mnt_mounts);
}

static void commit_tree(struct mount *mnt)
{
    struct mount *parent = mnt->mnt_parent;

    BUG_ON(parent == mnt);

    __attach_mnt(mnt, parent);
}

static struct mountpoint *unhash_mnt(struct mount *mnt)
{
    struct mountpoint *mp;
    mnt->mnt_parent = mnt;
    mnt->mnt_mountpoint = mnt->mnt.mnt_root;
    list_del_init(&mnt->mnt_child);
    hlist_del_init(&mnt->mnt_hash);
    hlist_del_init(&mnt->mnt_mp_list);
    mp = mnt->mnt_mp;
    mnt->mnt_mp = NULL;
    return mp;
}

static void
attach_mnt(struct mount *mnt,
           struct mount *parent,
           struct mountpoint *mp)
{
    mnt_set_mountpoint(parent, mp, mnt);
    __attach_mnt(mnt, parent);
}

static int
attach_recursive_mnt(struct mount *source_mnt,
                     struct mount *dest_mnt,
                     struct mountpoint *dest_mp,
                     bool moving)
{
    struct mountpoint *smp;

    /* Preallocate a mountpoint in case the new mounts need
     * to be tucked under other mounts.
     */
    smp = get_mountpoint(source_mnt->mnt.mnt_root);
    if (IS_ERR(smp))
        return PTR_ERR(smp);

    if (moving) {
        unhash_mnt(source_mnt);
        attach_mnt(source_mnt, dest_mnt, dest_mp);
    } else {
        mnt_set_mountpoint(dest_mnt, dest_mp, source_mnt);
        commit_tree(source_mnt);
    }

    return 0;
}

static void unlock_mount(struct mountpoint *where)
{
    /* Todo */
}

static int
graft_tree(struct mount *mnt, struct mount *p, struct mountpoint *mp)
{
    if (mnt->mnt.mnt_sb->s_flags & SB_NOUSER)
        return -EINVAL;

    return attach_recursive_mnt(mnt, p, mp, false);
}

/*
 * add a mount into a namespace's mount tree
 */
static int do_add_mount(struct mount *newmnt, struct mountpoint *mp,
                        struct path *path, int mnt_flags)
{
    struct mount *parent = real_mount(path->mnt);

    /* Refuse the same filesystem on the same mount point */
    if (path->mnt->mnt_sb == newmnt->mnt.mnt_sb &&
        path->mnt->mnt_root == path->dentry)
        return -EBUSY;

    return graft_tree(newmnt, parent, mp);
}

/*
 * create a new mount for userspace and request it to be added into the
 * namespace's tree
 */
static int
do_new_mount_fc(struct fs_context *fc, struct path *mountpoint,
                unsigned int mnt_flags)
{
    int error;
    struct vfsmount *mnt;
    struct mountpoint *mp;

    mnt = vfs_create_mount(fc);
    if (IS_ERR(mnt))
        return PTR_ERR(mnt);

    mp = lock_mount(mountpoint);
    if (IS_ERR(mp))
        return PTR_ERR(mp);

    error = do_add_mount(real_mount(mnt), mp, mountpoint, mnt_flags);
    unlock_mount(mp);
    return error;
}

static int
do_new_mount(struct path *path,
             const char *fstype,
             int sb_flags,
             int mnt_flags,
             const char *name)
{
    int err;
    struct fs_context *fc;
    struct file_system_type *type;

    if (!fstype)
        return -EINVAL;

    type = get_fs_type(fstype);
    if (!type)
        return -ENODEV;

    BUG_ON(type->fs_flags & FS_HAS_SUBTYPE);

    fc = fs_context_for_mount(type, sb_flags);
    if (IS_ERR(fc))
        return PTR_ERR(fc);

    if (name)
        err = vfs_parse_fs_string(fc, "source", name, strlen(name));

    if (!err)
        err = vfs_get_tree(fc);

    printk("%s: type(%s) path(%s) source(%s)\n",
           __func__, type->name, path->dentry->d_name.name, fc->source);
    if (!err)
        err = do_new_mount_fc(fc, path, mnt_flags);

    put_fs_context(fc);
    return err;
}

static int do_move_mount(struct path *old_path, struct path *new_path)
{
    int err;
    bool attached;
    struct mount *p;
    struct mount *old;
    struct mount *parent;
    struct mountpoint *mp, *old_mp;

    mp = lock_mount(new_path);
    if (IS_ERR(mp))
        return PTR_ERR(mp);

    old = real_mount(old_path->mnt);
    p = real_mount(new_path->mnt);
    parent = old->mnt_parent;
    attached = mnt_has_parent(old);
    old_mp = old->mnt_mp;

    err = attach_recursive_mnt(old, real_mount(new_path->mnt),
                               mp, attached);
    if (err)
        return err;

    unlock_mount(mp);
    return err;
}

static int do_move_mount_old(struct path *path, const char *old_name)
{
    struct path old_path;
    int err;

    if (!old_name || !*old_name)
        return -EINVAL;

    err = kern_path(old_name, LOOKUP_FOLLOW, &old_path);
    if (err)
        return err;

    err = do_move_mount(&old_path, path);
    path_put(&old_path);
    return err;
}

int
path_mount(const char *dev_name, struct path *path,
           const char *type_page, unsigned long flags)
{
    unsigned int sb_flags;
    unsigned int mnt_flags = 0;

    if (flags & MS_RDONLY)
        mnt_flags |= MNT_READONLY;

    sb_flags = flags & (SB_RDONLY | SB_SYNCHRONOUS | SB_MANDLOCK | SB_DIRSYNC |
                        SB_SILENT | SB_POSIXACL | SB_LAZYTIME | SB_I_VERSION);

    if (flags & MS_MOVE)
        return do_move_mount_old(path, dev_name);

    return do_new_mount(path, type_page, sb_flags, mnt_flags, dev_name);
}

int
init_mount(const char *dev_name, const char *dir_name,
           const char *type_page, unsigned long flags)
{
    struct path path;
    int ret;

    ret = kern_path(dir_name, LOOKUP_FOLLOW, &path);
    if (ret)
        return ret;

    ret = path_mount(dev_name, &path, type_page, flags);
    path_put(&path);
    return ret;
}
EXPORT_SYMBOL(init_mount);

static struct mount *
alloc_vfsmnt(const char *name)
{
    struct mount *mnt;
    mnt = kmem_cache_zalloc(mnt_cache, GFP_KERNEL);
    if (mnt) {
        INIT_HLIST_NODE(&mnt->mnt_hash);
        INIT_LIST_HEAD(&mnt->mnt_child);
        INIT_LIST_HEAD(&mnt->mnt_mounts);
        INIT_HLIST_NODE(&mnt->mnt_mp_list);
    }
    return mnt;
}

struct vfsmount *
vfs_create_mount(struct fs_context *fc)
{
    struct mount *mnt;

    if (!fc->root)
        return ERR_PTR(-EINVAL);

    mnt = alloc_vfsmnt("none");
    if (!mnt)
        return ERR_PTR(-ENOMEM);

    mnt->mnt.mnt_sb   = fc->root->d_sb;
    mnt->mnt.mnt_root = dget(fc->root);
    return &mnt->mnt;
}

struct vfsmount *
fc_mount(struct fs_context *fc)
{
    int err = vfs_get_tree(fc);
    if (err) {
        return ERR_PTR(err);
    }
    return vfs_create_mount(fc);
}
EXPORT_SYMBOL(fc_mount);

struct vfsmount *
vfs_kern_mount(struct file_system_type *type,
               int flags, const char *name, void *data)
{
    struct fs_context *fc;
    struct vfsmount *mnt;
    int ret = 0;

    if (!type)
        return ERR_PTR(-EINVAL);

    fc = fs_context_for_mount(type, flags);
    if (IS_ERR(fc))
        return ERR_CAST(fc);

    mnt = fc_mount(fc);

    put_fs_context(fc);
    return mnt;
}
EXPORT_SYMBOL(vfs_kern_mount);

struct vfsmount *
kern_mount(struct file_system_type *type)
{
    return vfs_kern_mount(type, SB_KERNMOUNT, type->name, NULL);
}
EXPORT_SYMBOL(kern_mount);

void *copy_mount_options(const void *data)
{
    char *copy;
    unsigned size;

    if (!data)
        return NULL;

    copy = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!copy)
        return ERR_PTR(-ENOMEM);

    size = PAGE_SIZE - offset_in_page(data);

    if (copy_from_user(copy, data, size)) {
        kfree(copy);
        return ERR_PTR(-EFAULT);
    }
    if (size != PAGE_SIZE) {
        if (copy_from_user(copy + size, data + size, PAGE_SIZE - size))
            memset(copy + size, 0, PAGE_SIZE - size);
    }
    return copy;
}

char *copy_mount_string(const void *data)
{
    return data ? strndup_user(data, PATH_MAX) : NULL;
}

long
do_mount(const char *dev_name, const char *dir_name,
         const char *type_page, unsigned long flags, void *data_page)
{
    int ret;
    struct path path;

    ret = user_path_at(AT_FDCWD, dir_name, LOOKUP_FOLLOW, &path);
    if (ret)
        return ret;
    ret = path_mount(dev_name, &path, type_page, flags);
    path_put(&path);
    return ret;
}

long _do_sys_mount(char *dev_name, char *dir_name, char *type,
                   unsigned long flags, void *data)
{
    int ret;
    char *kernel_type;
    char *kernel_dev;
    void *options;

    kernel_type = copy_mount_string(type);
    ret = PTR_ERR(kernel_type);
    if (IS_ERR(kernel_type))
        return ret;

    kernel_dev = copy_mount_string(dev_name);
    ret = PTR_ERR(kernel_dev);
    if (IS_ERR(kernel_dev))
        return ret;

    options = copy_mount_options(data);
    ret = PTR_ERR(options);
    if (IS_ERR(options))
        return ret;

    return do_mount(kernel_dev, dir_name, kernel_type, flags, options);
}

void
mnt_init(void)
{
    do_sys_mount = _do_sys_mount;

    mnt_cache = kmem_cache_create("mnt_cache", sizeof(struct mount),0,
                                  SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);

    mount_hashtable =
        alloc_large_system_hash("Mount-cache", sizeof(struct hlist_head),
                                19, &m_hash_shift, &m_hash_mask);

    mountpoint_hashtable =
        alloc_large_system_hash("Mountpoint-cache",
                                sizeof(struct hlist_head),
                                19, &mp_hash_shift, &mp_hash_mask);
}

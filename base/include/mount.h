/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MOUNT_H
#define _LINUX_MOUNT_H

#include <fs.h>
#include <stat.h>
#include <dcache.h>
#include <kdev_t.h>

#define MS_RDONLY   1       /* Mount read-only */
#define MS_MOVE     8192
#define MS_SILENT   32768

#define MNT_READONLY    0x40    /* does the user want this to be r/o? */
#define MNT_LOCKED      0x800000

struct vfsmount {
    struct dentry *mnt_root;    /* root of the mounted tree */
    struct super_block *mnt_sb; /* pointer to superblock */
};

struct mountpoint {
    struct hlist_node m_hash;
    struct dentry *m_dentry;
    struct hlist_head m_list;
    int m_count;
};

struct mount {
    struct hlist_node mnt_hash;
    struct mount *mnt_parent;
    struct dentry *mnt_mountpoint;
    struct list_head mnt_mounts;    /* list of children, anchored here */
    struct list_head mnt_child;     /* and going through their mnt_child */
    struct vfsmount mnt;
    struct mountpoint *mnt_mp;      /* where is it mounted */
    union {
        struct hlist_node mnt_mp_list;  /* list mounts with the same mountpoint */
        struct hlist_node mnt_umount;
    };
};

void
mnt_init(void);

int
init_mknod(const char *filename, umode_t mode, unsigned int dev);

static inline int
create_dev(char *name, dev_t dev)
{
    return init_mknod(name, S_IFBLK|0600, new_encode_dev(dev));
}

int
init_mount(const char *dev_name, const char *dir_name,
           const char *type_page, unsigned long flags);

int
kern_path(const char *name, unsigned int flags, struct path *path);

struct vfsmount *
kern_mount(struct file_system_type *type);

static inline struct mount *
real_mount(struct vfsmount *mnt)
{
    return container_of(mnt, struct mount, mnt);
}

struct vfsmount *
vfs_create_mount(struct fs_context *fc);

static inline int mnt_has_parent(struct mount *mnt)
{
    return mnt != mnt->mnt_parent;
}

int
path_mount(const char *dev_name, struct path *path,
           const char *type_page, unsigned long flags);

#endif /* _LINUX_MOUNT_H */

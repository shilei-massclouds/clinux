/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_NAMEI_H
#define _LINUX_NAMEI_H

#include <fs.h>
#include <dcache.h>

/*
 * Type of the last component on LOOKUP_PARENT
 */
enum {LAST_NORM, LAST_ROOT, LAST_DOT, LAST_DOTDOT};

/* Special value used to indicate
 * openat should use the current working directory. */
#define AT_FDCWD    -100

#define LOOKUP_FOLLOW       0x0001  /* follow links at the end */
#define LOOKUP_DIRECTORY    0x0002  /* require a directory */
#define LOOKUP_ROOT_GRABBED 0x0008
#define LOOKUP_PARENT       0x0010  /* internal use only */
#define LOOKUP_REVAL        0x0020  /* tell ->d_revalidate() to trust no cache */
#define LOOKUP_RCU          0x0040  /* RCU pathwalk mode; semi-internal */
#define LOOKUP_OPEN         0x0100  /* ... in open */
#define LOOKUP_CREATE       0x0200  /* ... in object creation */
#define LOOKUP_EXCL         0x0400  /* ... in exclusive creation */
#define LOOKUP_JUMPED       0x1000
#define LOOKUP_ROOT         0x2000
#define LOOKUP_EMPTY        0x4000  /* accept empty path [user_... only] */

/* Scoping flags for lookup. */
#define LOOKUP_NO_SYMLINKS      0x010000 /* No symlink crossing. */
#define LOOKUP_NO_MAGICLINKS    0x020000 /* No nd_jump_link() crossing. */
#define LOOKUP_NO_XDEV          0x040000 /* No mountpoint crossing. */
#define LOOKUP_IN_ROOT          0x100000 /* Treat dirfd as fs root. */
#define LOOKUP_BENEATH          0x080000 /* No escaping from starting point. */

struct dentry *
kern_path_create(int dfd, const char *pathname,
                 struct path *path, unsigned int lookup_flags);

int
vfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);

int
vfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev);

int
kern_path(const char *name, unsigned int flags, struct path *path);

struct filename *getname_kernel(const char *filename);

int
user_path_at_empty(int dfd, const char *name, unsigned flags,
                   struct path *path, int *empty);

static inline int
user_path_at(int dfd, const char *name, unsigned flags,
             struct path *path)
{
    return user_path_at_empty(dfd, name, flags, path, NULL);
}

struct filename *
getname_flags(const char *filename, int flags, int *empty);

#endif /* _LINUX_NAMEI_H */

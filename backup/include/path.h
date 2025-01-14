/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PATH_H_
#define _LINUX_PATH_H_

struct vfsmount;
struct dentry;

struct path {
    struct vfsmount *mnt;
    struct dentry *dentry;
};

static inline void
path_put(const struct path *path)
{
}

#endif /* _LINUX_PATH_H_ */

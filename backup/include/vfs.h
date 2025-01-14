/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _VFS_H_
#define _VFS_H_

#include <fs.h>

struct vfsmount *
vfs_kern_mount(struct file_system_type *type, int flags,
               const char *name, void *data);

#endif /* _VFS_H_ */

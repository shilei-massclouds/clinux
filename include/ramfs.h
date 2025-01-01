/* SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0 */
#ifndef _RAMFS_H_
#define _RAMFS_H_

#include <fs.h>

int
ramfs_init_fs_context(struct fs_context *fc);

struct inode *
ramfs_get_inode(struct super_block *sb, const struct inode *dir,
                umode_t mode, dev_t dev);

#endif /* _RAMFS_H_ */

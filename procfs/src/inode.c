/* SPDX-License-Identifier: GPL-2.0-only */

#include <fs.h>

#include "internal.h"

struct inode *
proc_get_inode(struct super_block *sb, struct proc_dir_entry *de)
{
    struct inode *inode = new_inode(sb);
    return inode;
}

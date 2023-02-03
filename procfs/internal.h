/* SPDX-License-Identifier: GPL-2.0-or-later */

struct proc_dir_entry {
    char *name;
};

struct inode *
proc_get_inode(struct super_block *sb, struct proc_dir_entry *de);

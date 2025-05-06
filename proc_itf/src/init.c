// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/seq_file.h>
/*
#include <linux/fs.h>
#include <linux/jump_label.h>
#include <cl_hook.h>
*/
#include "../../booter/src/booter.h"

int
cl_proc_itf_init(void)
{
    sbi_puts("module[proc_itf]: init begin ...\n");
    sbi_puts("module[proc_itf]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_proc_itf_init);

__weak struct proc_dir_entry *
proc_create_seq_private(const char *name,
                        umode_t mode,
                        struct proc_dir_entry *parent,
                        const struct seq_operations *ops,
                        unsigned int state_size,
                        void *data)
{
}
EXPORT_SYMBOL(proc_create_seq_private);

/*
const struct file_operations proc_fd_operations;
const struct file_operations proc_pid_smaps_rollup_operations;
const struct file_operations proc_clear_refs_operations;
const struct file_operations proc_tid_children_operations;
const struct file_operations proc_fd_inode_operations;
const struct file_operations proc_ns_dir_inode_operations;
const struct file_operations proc_fdinfo_inode_operations;
const struct file_operations proc_fdinfo_operations;
const struct file_operations proc_ns_dir_operations;
const struct file_operations proc_mounts_operations;
const struct file_operations proc_pid_maps_operations;
const struct file_operations proc_pagemap_operations;
const struct file_operations proc_mountinfo_operations;
const struct file_operations proc_mountstats_operations;
const struct file_operations proc_pid_smaps_operations;

// From kernel/bpf/cgroup.c
DEFINE_STATIC_KEY_FALSE(cgroup_bpf_enabled_key);
EXPORT_SYMBOL(cgroup_bpf_enabled_key);
*/

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/jump_label.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_proc_init(void)
{
    sbi_puts("module[proc]: init begin ...\n");
    sbi_puts("module[proc]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_proc_init);

DEFINE_ENABLE_FUNC(proc);

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

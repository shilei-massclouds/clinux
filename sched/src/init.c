// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernfs.h>
#include <linux/hrtimer.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/cgroup.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_sched_init(void)
{
    sbi_puts("module[sched]: init begin ...\n");
    sbi_puts("module[sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_sched_init);

__weak struct task_struct *__switch_to(struct task_struct *,
                       struct task_struct *)
{
    booter_panic("No impl in 'sched'.");
}
EXPORT_SYMBOL(__switch_to);

/*
struct dentry *debugfs_create_bool(const char *name, umode_t mode,
                   struct dentry *parent, bool *value)
{
    booter_panic("No impl 'sched'.");
}
*/

void io_wq_worker_running(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

void io_wq_worker_sleeping(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

/*
int proc_dointvec(struct ctl_table *table, int write, void *buffer,
          size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}
*/

/*
void rcu_qs(void)
{
    booter_panic("No impl 'sched'.");
}

struct proc_dir_entry *proc_create_seq_private(const char *name, umode_t mode,
        struct proc_dir_entry *parent, const struct seq_operations *ops,
        unsigned int state_size, void *data)
{
    booter_panic("No impl 'sched'.");
}
*/

/*
void rt_mutex_adjust_pi(struct task_struct *task)
{
    booter_panic("No impl in 'sched'.");
}
*/

/*
void __cgroup_account_cputime_field(struct cgroup *cgrp,
                    enum cpu_usage_stat index, u64 delta_exec)
{
    booter_panic("No impl in 'sched'.");
}
*/

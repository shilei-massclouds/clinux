// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernfs.h>
#include <linux/hrtimer.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
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

int kernfs_path_from_node(struct kernfs_node *to, struct kernfs_node *from,
              char *buf, size_t buflen)
{
    booter_panic("No impl 'sched'.");
}

void __put_task_struct(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

void hrtimer_init(struct hrtimer *timer, clockid_t clock_id,
          enum hrtimer_mode mode)
{
    booter_panic("No impl 'sched'.");
}

void __init generic_sched_clock_init(void)
{
    booter_panic("No impl 'sched'.");
}

void touch_all_softlockup_watchdogs(void)
{
    booter_panic("No impl 'sched'.");
}

loff_t seq_lseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl 'sched'.");
}

pid_t __task_pid_nr_ns(struct task_struct *task, enum pid_type type,
            struct pid_namespace *ns)
{
    booter_panic("No impl 'sched'.");
}

struct dentry *debugfs_create_bool(const char *name, umode_t mode,
                   struct dentry *parent, bool *value)
{
    booter_panic("No impl 'sched'.");
}

void io_wq_worker_running(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

void switch_mm(struct mm_struct *prev, struct mm_struct *next,
    struct task_struct *task)
{
    booter_panic("No impl 'sched'.");
}

void put_task_struct_rcu_user(struct task_struct *task)
{
    booter_panic("No impl 'sched'.");
}

/*
void print_modules(void)
{
    booter_panic("No impl 'sched'.");
}
*/

bool kthread_should_stop(void)
{
    booter_panic("No impl 'sched'.");
}

struct task_struct *__switch_to(struct task_struct *,
                       struct task_struct *)
{
    booter_panic("No impl 'sched'.");
}

signed long __sched schedule_timeout(signed long timeout)
{
    booter_panic("No impl 'sched'.");
}

void put_task_stack(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

void io_wq_worker_sleeping(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}

void tick_nohz_idle_restart_tick(void)
{
    booter_panic("No impl 'sched'.");
}

void tick_nohz_idle_stop_tick(void)
{
    booter_panic("No impl 'sched'.");
}

/*
void touch_softlockup_watchdog(void)
{
    booter_panic("No impl 'sched'.");
}
*/

void __mmdrop(struct mm_struct *mm)
{
    booter_panic("No impl 'sched'.");
}

ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}

void seq_puts(struct seq_file *m, const char *s)
{
    booter_panic("No impl 'sched'.");
}

void seq_printf(struct seq_file *m, const char *f, ...)
{
    booter_panic("No impl 'sched'.");
}

/*
int printk_deferred(const char *s, ...)
{
    booter_panic("No impl 'sched'.");
}
*/

void tick_nohz_idle_exit(void)
{
    booter_panic("No impl 'sched'.");
}

struct dentry *debugfs_create_file(const char *name, umode_t mode,
                   struct dentry *parent, void *data,
                   const struct file_operations *fops)
{
    booter_panic("No impl 'sched'.");
}

void tick_nohz_idle_enter(void)
{
    booter_panic("No impl 'sched'.");
}

void __cgroup_account_cputime(struct cgroup *cgrp, u64 delta_exec)
{
    booter_panic("No impl 'sched'.");
}

/*
ktime_t ktime_get(void)
{
    booter_panic("No impl 'sched'.");
}
*/

void blk_flush_plug_list(struct blk_plug *plug, bool from_schedule)
{
    booter_panic("No impl 'sched'.");
}

int proc_dointvec(struct ctl_table *table, int write, void *buffer,
          size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}

int single_release(struct inode *inode, struct file *file)
{
    booter_panic("No impl 'sched'.");
}

u64 hrtimer_forward(struct hrtimer *timer, ktime_t now, ktime_t interval)
{
    booter_panic("No impl 'sched'.");
}

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

int single_open(struct file *file, int (*show)(struct seq_file *, void *),
        void *data)
{
    booter_panic("No impl 'sched'.");
}

int proc_dointvec_minmax(struct ctl_table *table, int write,
          void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}


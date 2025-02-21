// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/sched/debug.h>
#include <linux/sched/wake_q.h>
#include "sched.h"
#include "../../booter/src/booter.h"

int
cl_early_sched_init(void)
{
    sbi_puts("module[early_sched]: init begin ...\n");
    sbi_puts("module[early_sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_sched_init);

void __weak set_user_nice(struct task_struct *p, long nice)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("set_user_nice");
    }
}
EXPORT_SYMBOL(set_user_nice);

void __sched __weak wait_for_completion(struct completion *x)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("wait_for_completion");
    }
}
EXPORT_SYMBOL(wait_for_completion);

#ifndef CONFIG_PREEMPTION
int __sched _cond_resched(void)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("_cond_resched");
    }
    return 0;
}
EXPORT_SYMBOL(_cond_resched);
#endif

#ifdef CONFIG_DEBUG_ATOMIC_SLEEP
void __might_sleep(const char *file, int line, int preempt_offset)
{
    ___might_sleep(file, line, preempt_offset);
}
EXPORT_SYMBOL(__might_sleep);

void ___might_sleep(const char *file, int line, int preempt_offset)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("___might_sleep");
    }
}
EXPORT_SYMBOL(___might_sleep);
#endif

void wake_q_add(struct wake_q_head *head, struct task_struct *task)
{
    booter_panic("in mutex!");
}
EXPORT_SYMBOL(wake_q_add);

void wake_up_q(struct wake_q_head *head)
{
    booter_panic("in mutex!");
}
EXPORT_SYMBOL(wake_up_q);

void print_modules(void)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(print_modules);

void __weak io_schedule_finish(int token)
{
    booter_panic("in mutex!");
}
EXPORT_SYMBOL(io_schedule_finish);

int __weak io_schedule_prepare(void)
{
    booter_panic("in mutex!");
}
EXPORT_SYMBOL(io_schedule_prepare);

void __weak __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
            int nr_exclusive, void *key)
{
    booter_panic("No impl '__wake_up'.");
}
EXPORT_SYMBOL(__wake_up);

asmlinkage __visible __weak void __sched schedule(void)
{
    booter_panic("No impl 'rwsem'.");
}
EXPORT_SYMBOL(schedule);

signed long __sched __weak schedule_timeout(signed long timeout)
{
    booter_panic("No impl 'schedule_timeout'.");
}
EXPORT_SYMBOL(schedule_timeout);

void __weak __init_swait_queue_head(struct swait_queue_head *q, const char *name,
			     struct lock_class_key *key)
{
    booter_panic("No impl in 'early_sched'.");
}
EXPORT_SYMBOL(__init_swait_queue_head);

notrace void touch_softlockup_watchdog(void)
{
    booter_panic("No impl 'touch_softlockup_watchdog'.");
}
EXPORT_SYMBOL(touch_softlockup_watchdog);

void __weak complete(struct completion *x)
{
    booter_panic("No impl 'touch_softlockup_watchdog'.");
}
EXPORT_SYMBOL(complete);

int __weak printk_deferred(const char *s, ...)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(printk_deferred);

bool kthread_should_stop(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_should_stop);

signed long __sched __weak schedule_timeout_uninterruptible(signed long timeout)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(schedule_timeout_uninterruptible);

void __weak calc_global_load(void)
{
    booter_panic("No impl in 'early_sched'.");
}
EXPORT_SYMBOL(calc_global_load);

void __weak calc_load_nohz_start(void)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(calc_load_nohz_start);

void __weak calc_load_nohz_stop(void)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(calc_load_nohz_stop);

void __weak swake_up_one(struct swait_queue_head *q)
{
    booter_panic("No impl in 'early_sched'.");
}
EXPORT_SYMBOL(swake_up_one);

long __weak prepare_to_swait_event(struct swait_queue_head *q, struct swait_queue *wait, int state)
{
    booter_panic("No impl in 'early_sched'.");
}
EXPORT_SYMBOL(prepare_to_swait_event);

void __weak finish_swait(struct swait_queue_head *q, struct swait_queue *wait)
{
    booter_panic("No impl in 'early_sched'.");
}
EXPORT_SYMBOL(finish_swait);

__weak int kernfs_path_from_node(struct kernfs_node *to, struct kernfs_node *from,
              char *buf, size_t buflen)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(kernfs_path_from_node);

__weak void
prepare_to_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(prepare_to_wait);

__weak void finish_wait(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(finish_wait);

__weak int __sched out_of_line_wait_on_bit(void *word, int bit,
				    wait_bit_action_f *action, unsigned mode)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(out_of_line_wait_on_bit);

__weak __sched int bit_wait(struct wait_bit_key *word, int mode)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(bit_wait);

__weak void __wake_up_bit(struct wait_queue_head *wq_head, void *word, int bit)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__wake_up_bit);

__weak void wake_up_bit(void *word, int bit)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(wake_up_bit);

__weak long prepare_to_wait_event(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(prepare_to_wait_event);

__weak void init_wait_entry(struct wait_queue_entry *wq_entry, int flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(init_wait_entry);

__weak int idle_cpu(int cpu)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(idle_cpu);

__weak void sched_set_fifo(struct task_struct *p)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(sched_set_fifo);

DEFINE_PER_CPU(struct kernel_stat, kstat);
DEFINE_PER_CPU(struct kernel_cpustat, kernel_cpustat);

EXPORT_PER_CPU_SYMBOL(kstat);
EXPORT_PER_CPU_SYMBOL(kernel_cpustat);

__weak wait_queue_head_t *bit_waitqueue(void *word, int bit)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(bit_waitqueue);

__weak int wake_bit_function(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *arg)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(wake_bit_function);

__weak int autoremove_wake_function(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *key)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(autoremove_wake_function);

__weak void add_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(add_wait_queue);

__weak void __wake_up_locked(struct wait_queue_head *wq_head, unsigned int mode, int nr)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(__wake_up_locked);

__weak void __sched io_schedule(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(io_schedule);

__weak void __wake_up_locked_key_bookmark(struct wait_queue_head *wq_head,
		unsigned int mode, void *key, wait_queue_entry_t *bookmark)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(__wake_up_locked_key_bookmark);

__weak int wake_up_state(struct task_struct *p, unsigned int state)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(wake_up_state);

__weak long __sched io_schedule_timeout(long timeout)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(io_schedule_timeout);

__weak int default_wake_function(wait_queue_entry_t *curr, unsigned mode, int wake_flags,
			  void *key)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(default_wake_function);

__weak void proc_sched_set_task(struct task_struct *p)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(proc_sched_set_task);

__weak void proc_sched_show_task(struct task_struct *p, struct pid_namespace *ns,
						  struct seq_file *m)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(proc_sched_show_task);

__weak long wait_woken(struct wait_queue_entry *wq_entry, unsigned mode, long timeout)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(wait_woken);

__weak int woken_wake_function(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *key)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(woken_wake_function);

__weak void remove_wait_queue(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(remove_wait_queue);

__weak void init_wait_var_entry(struct wait_bit_queue_entry *wbq_entry, void *var, int flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(init_wait_var_entry);

__weak wait_queue_head_t *__var_waitqueue(void *p)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__var_waitqueue);

__weak void __cgroup_account_cputime_field(struct cgroup *cgrp,
				    enum cpu_usage_stat index, u64 delta_exec)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__cgroup_account_cputime_field);

__weak void __cgroup_account_cputime(struct cgroup *cgrp, u64 delta_exec)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(__cgroup_account_cputime);

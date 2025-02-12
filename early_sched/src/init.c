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

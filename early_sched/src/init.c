// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/sched/debug.h>
#include <linux/sched/wake_q.h>
#include "../../booter/src/booter.h"

int
cl_early_sched_init(void)
{
    sbi_puts("module[early_sched]: init begin ...\n");
    sbi_puts("module[early_sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_sched_init);

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

notrace void touch_softlockup_watchdog(void)
{
    booter_panic("No impl 'touch_softlockup_watchdog'.");
}
EXPORT_SYMBOL(touch_softlockup_watchdog);

int __weak printk_deferred(const char *s, ...)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(printk_deferred);

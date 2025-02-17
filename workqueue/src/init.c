// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/rcuwait.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_workqueue_init(void)
{
    sbi_puts("module[workqueue]: init begin ...\n");
    sbi_puts("module[workqueue]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_workqueue_init);

/*
void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}
*/

/*
void add_timer(struct timer_list *timer)
{
    booter_panic("No impl in 'workqueue'.");
}

void add_timer_on(struct timer_list *timer, int cpu)
{
    booter_panic("No impl in 'workqueue'.");
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{
    booter_panic("No impl in 'workqueue'.");
}

int kthread_stop(struct task_struct *k)
{
    booter_panic("No impl in 'workqueue'.");
}
*/

/*
unsigned int jiffies_to_msecs(const unsigned long j)
{
    booter_panic("No impl in 'workqueue'.");
}

int device_register(struct device *dev)
{
    booter_panic("No impl in 'workqueue'.");
}
*/

void kthread_bind_mask(struct task_struct *p, const struct cpumask *mask)
{
    booter_panic("No impl in 'workqueue'.");
}

/*
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data, int node,
                       const char namefmt[],
                       ...)
{
    booter_panic("No impl in 'workqueue'.");
}
*/

/*
void init_timer_key(struct timer_list *timer,
            void (*func)(struct timer_list *), unsigned int flags,
            const char *name, struct lock_class_key *key)
{
    booter_panic("No impl in 'workqueue'.");
}
*/

int bitmap_parse(const char *start, unsigned int buflen,
        unsigned long *maskp, int nmaskbits)
{
    booter_panic("No impl in 'workqueue'.");
}

int rcuwait_wake_up(struct rcuwait *w)
{
    booter_panic("No impl in 'workqueue'.");
}

void __set_task_comm(struct task_struct *tsk, const char *buf, bool exec)
{
    booter_panic("No impl in 'workqueue'.");
}

void *kthread_probe_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include "../../booter/src/booter.h"

int
cl_workqueue_itf_init(void)
{
    sbi_puts("module[workqueue_itf]: init begin ...\n");
    sbi_puts("module[workqueue_itf]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_workqueue_itf_init);

struct workqueue_struct *system_wq __read_mostly;
EXPORT_SYMBOL(system_wq);
struct workqueue_struct *system_highpri_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_highpri_wq);
struct workqueue_struct *system_long_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_long_wq);
struct workqueue_struct *system_unbound_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_unbound_wq);
struct workqueue_struct *system_freezable_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_freezable_wq);
struct workqueue_struct *system_power_efficient_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_power_efficient_wq);
struct workqueue_struct *system_freezable_power_efficient_wq __read_mostly;
EXPORT_SYMBOL_GPL(system_freezable_power_efficient_wq);

bool __weak queue_work_on(int cpu, struct workqueue_struct *wq,
		   struct work_struct *work)
{
    booter_panic("No impl in 'workqueue_itf'.");
}
EXPORT_SYMBOL(queue_work_on);

bool __weak queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
			   struct delayed_work *dwork, unsigned long delay)
{
    booter_panic("No impl in 'workqueue_itf'.");
}
EXPORT_SYMBOL(queue_delayed_work_on);

void __weak delayed_work_timer_fn(struct timer_list *t)
{
    booter_panic("No impl in 'workqueue_itf'.");
}
EXPORT_SYMBOL(delayed_work_timer_fn);

bool __weak flush_work(struct work_struct *work)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(flush_work);

__weak bool mod_delayed_work_on(int cpu, struct workqueue_struct *wq,
			 struct delayed_work *dwork, unsigned long delay)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL_GPL(mod_delayed_work_on);

__weak void destroy_workqueue(struct workqueue_struct *wq)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL_GPL(destroy_workqueue);

__weak __printf(1, 4)
struct workqueue_struct *alloc_workqueue(const char *fmt,
					 unsigned int flags,
					 int max_active, ...)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL_GPL(alloc_workqueue);

__weak void flush_workqueue(struct workqueue_struct *wq)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(flush_workqueue);

__weak bool queue_work_node(int node, struct workqueue_struct *wq,
		     struct work_struct *work)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL_GPL(queue_work_node);

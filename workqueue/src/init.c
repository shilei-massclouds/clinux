// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/idr.h>
#include "../../booter/src/booter.h"

int
cl_workqueue_init(void)
{
    sbi_puts("module[workqueue]: init begin ...\n");
    sbi_puts("module[workqueue]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_workqueue_init);

void *idr_find(const struct idr *idr, unsigned long id)
{
    booter_panic("No impl in 'workqueue'.");
}

void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}

void add_timer(struct timer_list *timer)
{
    booter_panic("No impl in 'workqueue'.");
}

void add_timer_on(struct timer_list *timer, int cpu)
{
    booter_panic("No impl in 'workqueue'.");
}

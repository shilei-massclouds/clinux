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

bool __weak queue_work_on(int cpu, struct workqueue_struct *wq,
		   struct work_struct *work)
{
    booter_panic("No impl 'page_alloc'.");
}
EXPORT_SYMBOL(queue_work_on);

bool __weak queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
			   struct delayed_work *dwork, unsigned long delay)
{
    booter_panic("No impl 'page_alloc'.");
}
EXPORT_SYMBOL(queue_delayed_work_on);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/irq_work.h>
#include <linux/wait.h>
#include "../../booter/src/booter.h"

int
cl_printk_init(void)
{
    sbi_puts("module[printk]: init begin ...\n");
    sbi_puts("module[printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_printk_init);

bool irq_work_queue(struct irq_work *work)
{
    booter_panic("No impl 'irq_work_queue'.");
}

/*
void __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
            int nr_exclusive, void *key)
{
    booter_panic("No impl '__wake_up'.");
}
*/

unsigned long long notrace sched_clock(void)
{
    booter_panic("No impl 'sched_clock'.");
}

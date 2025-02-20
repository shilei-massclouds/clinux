// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/fwnode.h>
#include <linux/task_work.h>
#include "../../booter/src/booter.h"

struct proc_dir_entry;

int
cl_irq_init(void)
{
    sbi_puts("module[irq]: init begin ...\n");
    sbi_puts("module[irq]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_irq_init);

void __weak __init irqchip_init(void)
{
    booter_panic("No impl in 'irq'.");
}
EXPORT_SYMBOL(irqchip_init);

struct callback_head *
task_work_cancel(struct task_struct *task, task_work_func_t func)
{
    booter_panic("No impl in 'irq'.");
}
void seq_putc(struct seq_file *m, char c)
{
    booter_panic("No impl in 'irq'.");
}

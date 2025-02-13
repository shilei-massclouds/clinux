// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/fwnode.h>
#include <linux/task_work.h>
#include "../../booter/src/booter.h"

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

/*
bool try_module_get(struct module *module)
{
    booter_panic("No impl in 'irq'.");
}

void __put_task_struct(struct task_struct *tsk)
{
    booter_panic("No impl in 'irq'.");
}
bool is_software_node(const struct fwnode_handle *fwnode)
{
    booter_panic("No impl in 'irq'.");
}
*/
/*
void add_interrupt_randomness(int irq, int irq_flags)
{
    booter_panic("No impl in 'irq'.");
}
*/
/*
void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl in 'irq'.");
}
*/

/*
void module_put(struct module *module)
{
    booter_panic("No impl in 'irq'.");
}
*/
int kobject_add(struct kobject *kobj, struct kobject *parent,
        const char *fmt, ...)
{
    booter_panic("No impl in 'irq'.");
}
struct callback_head *
task_work_cancel(struct task_struct *task, task_work_func_t func)
{
    booter_panic("No impl in 'irq'.");
}
void seq_putc(struct seq_file *m, char c)
{
    booter_panic("No impl in 'irq'.");
}
/*
int kthread_stop(struct task_struct *k)
{
    booter_panic("No impl in 'irq'.");
}
*/
struct proc_dir_entry *proc_mkdir(const char *name,
        struct proc_dir_entry *parent)
{
    booter_panic("No impl in 'irq'.");
}
/*
struct fwnode_handle *fwnode_handle_get(struct fwnode_handle *fwnode)
{
    booter_panic("No impl in 'irq'.");
}
*/
struct kobject *kobject_create_and_add(const char *name, struct kobject *parent)
{
    booter_panic("No impl in 'irq'.");
}
int
task_work_add(struct task_struct *task, struct callback_head *work, int notify)
{
    booter_panic("No impl in 'irq'.");
}
/*
void kobject_del(struct kobject *kobj)
{
    booter_panic("No impl in 'irq'.");
}
*/
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
{
    booter_panic("No impl in 'irq'.");
}
/*
void sched_set_fifo(struct task_struct *p)
{
    booter_panic("No impl in 'irq'.");
}
*/
struct proc_dir_entry *proc_create_single_data(const char *name, umode_t mode,
        struct proc_dir_entry *parent,
        int (*show)(struct seq_file *, void *), void *data)
{
    booter_panic("No impl in 'irq'.");
}
/*
void fwnode_handle_put(struct fwnode_handle *fwnode)
{
    booter_panic("No impl in 'irq'.");
}
*/
void proc_remove(struct proc_dir_entry *de)
{
    booter_panic("No impl in 'irq'.");
}
/*
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data, int node,
                       const char namefmt[],
                       ...)
{
    booter_panic("No impl in 'irq'.");
}
*/

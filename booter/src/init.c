// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/cache.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/sbi.h>
#include <asm/current.h>
#include <cl_hook.h>
#include "booter.h"

#define UL_STR_SIZE 19  /* prefix with '0x' and end with '\0' */

__visible u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

extern int cl_init();

void
start_kernel(void)
{
    cl_init();
    booter_panic("Kernel has been terminated normally!");
}

void sbi_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            sbi_console_putchar('\r');
        sbi_console_putchar(*s);
    }
}
EXPORT_SYMBOL(sbi_puts);

void sbi_put_u64(unsigned long n)
{
    char buf[UL_STR_SIZE];
    hex_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_u64);

void sbi_put_dec(unsigned long n)
{
    char buf[UL_STR_SIZE];
    dec_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_dec);

DEFINE_HOOK(int, vprintk_func, const char *fmt, va_list args);

//
// Hook for printk
//
asmlinkage __visible __weak int printk(const char *fmt, ...)
{
    if (HAS_HOOK(vprintk_func)) {
        int ret;
        va_list args;
        va_start(args, fmt);
        INVOKE_HOOK_RET(ret, vprintk_func, printk_skip_level(fmt), args);
        va_end(args);
        return ret;
    }

    // Fallthrough
    sbi_puts("[RAW] ");
    sbi_puts(printk_skip_level(fmt));
    return 0;
}
EXPORT_SYMBOL(printk);

void __warn_printk(const char *fmt, ...)
{
    if (HAS_HOOK(vprintk_func)) {
        int ret;
        va_list args;
        pr_warn(CUT_HERE);
        va_start(args, fmt);
        INVOKE_HOOK_RET(ret, vprintk_func, printk_skip_level(fmt), args);
        va_end(args);
        return;
    }

    // Fallthrough
    sbi_puts("[RAW_WARN] ");
    sbi_puts(printk_skip_level(fmt));
}
EXPORT_SYMBOL(__warn_printk);

//
// NOTE ************************
// Remove below definitions in future.
//

/**
 *  panic - halt the system
 *  @fmt: The text string to print
 *
 *  Display a message, then perform cleanups.
 *
 *  This function never returns.
 */
void __weak panic(const char *fmt, ...)
{
    sbi_puts("[RAW_PANIC] ");
    sbi_puts(fmt);
    sbi_puts("\n");
    sbi_shutdown();
    do {} while (1);
}
EXPORT_SYMBOL(panic);

void sysfs_remove_link(struct kobject *kobj, const char *name)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(sysfs_remove_link);

int kobject_uevent(struct kobject *kobj, enum kobject_action action)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobject_uevent);

void dev_printk(const char *level, const struct device *dev,
        const char *fmt, ...)
{
    sbi_puts("[RAW_DEV_PRINTK]:");
    sbi_puts(level);
    sbi_puts(" - ");
    sbi_puts(fmt);
    sbi_puts("\n");
}
EXPORT_SYMBOL(dev_printk);

unsigned long lpj_fine;
EXPORT_SYMBOL(lpj_fine);

void __local_bh_enable_ip(unsigned long ip, unsigned int cnt)
{
    booter_panic("No impl '__local_bh_enable_ip'.");
}
EXPORT_SYMBOL(__local_bh_enable_ip);

#ifndef __ARCH_IRQ_STAT
DEFINE_PER_CPU_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);
#endif

void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_data);

int kthread_stop(struct task_struct *k)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_stop);

struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data, int node,
                       const char namefmt[],
                       ...)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_create_on_node);

void __put_task_struct(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(__put_task_struct);

void kobject_del(struct kobject *kobj)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobject_del);

struct kobject *kernel_kobj;
EXPORT_SYMBOL_GPL(kernel_kobj);

void module_put(struct module *module)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(module_put);

int sysfs_create_link(struct kobject *kobj, struct kobject *target,
              const char *name)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(sysfs_create_link);

const struct sysfs_ops kobj_sysfs_ops;
EXPORT_SYMBOL(kobj_sysfs_ops);

#ifdef CONFIG_STACKPROTECTOR
#include <linux/stackprotector.h>
unsigned long __stack_chk_guard __read_mostly;
EXPORT_SYMBOL(__stack_chk_guard);
#endif

bool capable(int cap)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(capable);

int proc_dointvec(struct ctl_table *table, int write, void *buffer,
          size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(proc_dointvec);

bool __weak irq_work_needs_cpu(void)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(irq_work_needs_cpu);

void __weak irq_work_tick(void)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(irq_work_tick);

void kill_fasync(struct fasync_struct **fp, int sig, int band)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kill_fasync);

int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(fasync_helper);

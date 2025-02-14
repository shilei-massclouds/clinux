// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/ktime.h>
#include <linux/timerqueue.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/timekeeper_internal.h>
#include <linux/syscore_ops.h>
#include <linux/prandom.h>
#include "../../booter/src/booter.h"

int
cl_time_init(void)
{
    sbi_puts("module[time]: init begin ...\n");
    sbi_puts("module[time]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_time_init);

DEFINE_PER_CPU(struct rnd_state, net_rand_state)  __latent_entropy;
EXPORT_SYMBOL(net_rand_state);

/*
 * Scheduler clock - returns current time in nanosec units.
 * This is default implementation.
 * Architectures and sub-architectures can override this.
 */
unsigned long long __weak sched_clock(void)
{
	return (unsigned long long)(jiffies - INITIAL_JIFFIES)
					* (NSEC_PER_SEC / HZ);
}
EXPORT_SYMBOL_GPL(sched_clock);

bool timerqueue_del(struct timerqueue_head *head, struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_del'.");
}

struct timerqueue_node *timerqueue_iterate_next(struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_iterate_next'.");
}

bool timerqueue_add(struct timerqueue_head *head, struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_add'.");
}

void account_idle_ticks(unsigned long ticks)
{
    booter_panic("No impl in 'time'.");
}

int smp_call_function_single(int cpu, void (*func) (void *info), void *info,
                int wait)
{
    booter_panic("No impl in 'time'.");
}

void account_process_tick(struct task_struct *p, int user_tick)
{
    booter_panic("No impl in 'time'.");
}
/*
bool irq_work_needs_cpu(void)
{
    booter_panic("No impl in 'time'.");
}
*/

int rtc_set_ntp_time(struct timespec64 now, unsigned long *target_nsec)
{
    booter_panic("No impl in 'time'.");
}

loff_t seq_lseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(seq_lseek);

/*
bool capable(int cap)
{
    booter_panic("No impl in 'time'.");
}
void __init timer_probe(void)
{
    booter_panic("No impl in 'time'.");
}
*/

int cap_settime(const struct timespec64 *ts, const struct timezone *tz)
{
    booter_panic("No impl in 'time'.");
}

int device_register(struct device *dev)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(device_register);

void register_syscore_ops(struct syscore_ops *ops)
{
    booter_panic("No impl in 'time'.");
}
/*
void irq_work_tick(void)
{
    booter_panic("No impl in 'time'.");
}
*/
ssize_t seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(seq_read);
struct dentry *debugfs_create_file(const char *name, umode_t mode,
                   struct dentry *parent, void *data,
                   const struct file_operations *fops)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(debugfs_create_file);

void scheduler_tick(void)
{
    booter_panic("No impl in 'time'.");
}
void seq_printf(struct seq_file *m, const char *f, ...)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(seq_printf);
void seq_puts(struct seq_file *m, const char *s)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(seq_puts);
int single_release(struct inode *inode, struct file *file)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(single_release);
int single_open(struct file *file, int (*show)(struct seq_file *, void *),
        void *data)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(single_open);

void run_posix_cpu_timers(void)
{
    booter_panic("No impl in 'time'.");
}
int subsys_system_register(struct bus_type *subsys,
               const struct attribute_group **groups)
{
    booter_panic("No impl in 'time'.");
}
void hrtimers_resume(void)
{
    booter_panic("No impl in 'time'.");
}
int proc_dointvec_minmax(struct ctl_table *table, int write,
          void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(proc_dointvec_minmax);
void raise_softirq(unsigned int nr)
{
    booter_panic("No impl in 'time'.");
}

void on_each_cpu(smp_call_func_t func, void *info, int wait)
{
    booter_panic("No impl in 'time'.");
}

unsigned long nr_iowait_cpu(int cpu)
{
    booter_panic("No impl in 'time'.");
}
void timerfd_clock_was_set(void)
{
    booter_panic("No impl in 'time'.");
}
void touch_softlockup_watchdog_sched(void)
{
    booter_panic("No impl in 'time'.");
}

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

void __weak account_idle_ticks(unsigned long ticks)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(account_idle_ticks);

/*
int smp_call_function_single(int cpu, void (*func) (void *info), void *info,
                int wait)
{
    booter_panic("No impl in 'time'.");
}
*/

void __weak account_process_tick(struct task_struct *p, int user_tick)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(account_process_tick);

int rtc_set_ntp_time(struct timespec64 now, unsigned long *target_nsec)
{
    booter_panic("No impl in 'time'.");
}

struct dentry *debugfs_create_file(const char *name, umode_t mode,
                   struct dentry *parent, void *data,
                   const struct file_operations *fops)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(debugfs_create_file);

void __weak scheduler_tick(void)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(scheduler_tick);

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

/*
void on_each_cpu(smp_call_func_t func, void *info, int wait)
{
    booter_panic("No impl in 'time'.");
}
*/

unsigned long nr_iowait_cpu(int cpu)
{
    booter_panic("No impl in 'time'.");
}
void timerfd_clock_was_set(void)
{
    booter_panic("No impl in 'time'.");
}
/*
void touch_softlockup_watchdog_sched(void)
{
    booter_panic("No impl in 'time'.");
}
*/

void __weak thread_group_cputime(struct task_struct *tsk, struct task_cputime *times)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(thread_group_cputime);

int nanosleep_copyout(struct restart_block *restart, struct timespec64 *ts)
{
    booter_panic("No impl in 'time'.");
}

/*
pid_t __task_pid_nr_ns(struct task_struct *task, enum pid_type type,
            struct pid_namespace *ns)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(__task_pid_nr_ns);

struct task_struct *pid_task(struct pid *pid, enum pid_type type)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(pid_task);
*/

/*
*/

int posix_timer_event(struct k_itimer *timr, int si_private)
{
    booter_panic("No impl in 'time'.");
}

unsigned long long task_sched_runtime(struct task_struct *p)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(task_sched_runtime);

/*
struct pid *find_vpid(int nr)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(find_vpid);

void put_pid(struct pid *pid)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(put_pid);
*/

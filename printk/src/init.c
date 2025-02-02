// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/sched/debug.h>
#include <linux/irq_work.h>
#include <linux/cpuhotplug.h>
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

int __sched _cond_resched(void)
{
    booter_panic("No impl '_cond_resched'.");
}

void console_sysfs_notify(void)
{
    booter_panic("No impl 'console_sysfs_notify'.");
}

bool irq_work_queue(struct irq_work *work)
{
    booter_panic("No impl 'irq_work_queue'.");
}

void * __init memblock_alloc_try_nid(
            phys_addr_t size, phys_addr_t align,
            phys_addr_t min_addr, phys_addr_t max_addr,
            int nid)
{
    booter_panic("No impl 'memblock_alloc_try_nid'.");
}

void __might_sleep(const char *file, int line, int preempt_offset)
{
    booter_panic("No impl '__might_sleep'.");
}

void ___might_sleep(const char *file, int line, int preempt_offset)
{
    booter_panic("No impl '___might_sleep'.");
}

void __init n_tty_init(void)
{
    booter_panic("No impl 'n_tty_init'.");
}

notrace void touch_softlockup_watchdog(void)
{
    booter_panic("No impl 'touch_softlockup_watchdog'.");
}

void synchronize_rcu(void)
{
    booter_panic("No impl 'synchronize_rcu'.");
}

int __cpuhp_setup_state(enum cpuhp_state state,
            const char *name, bool invoke,
            int (*startup)(unsigned int cpu),
            int (*teardown)(unsigned int cpu),
            bool multi_instance)
{
    booter_panic("No impl '__cpuhp_setup_state'.");
}

void __wake_up(struct wait_queue_head *wq_head, unsigned int mode,
            int nr_exclusive, void *key)
{
    booter_panic("No impl '__wake_up'.");
}

unsigned long long notrace sched_clock(void)
{
    booter_panic("No impl 'sched_clock'.");
}

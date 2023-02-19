// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <ffs.h>
#include <export.h>
#include <printk.h>
#include <hardirq.h>

irq_cpustat_t irq_stat;
EXPORT_SYMBOL(irq_stat);

static struct softirq_action softirq_vec[NR_SOFTIRQS];

/**
 * irq_enter - Enter an interrupt context including RCU update
 */
void irq_enter(void)
{
    /* Todo */
}
EXPORT_SYMBOL(irq_enter);

static inline void invoke_softirq(void)
{
    do_softirq_own_stack();
}

static inline void __irq_exit_rcu(void)
{
    if (local_softirq_pending())
        invoke_softirq();
}

/**
 * irq_exit - Exit an interrupt context, update RCU and lockdep
 *
 * Also processes softirqs if needed and possible.
 */
void irq_exit(void)
{
    __irq_exit_rcu();
}
EXPORT_SYMBOL(irq_exit);

void open_softirq(int nr, void (*action)(struct softirq_action *))
{
    softirq_vec[nr].action = action;
}
EXPORT_SYMBOL(open_softirq);

void __raise_softirq_irqoff(unsigned int nr)
{
    or_softirq_pending(1UL << nr);
}

inline void raise_softirq_irqoff(unsigned int nr)
{
    __raise_softirq_irqoff(nr);
}
EXPORT_SYMBOL(raise_softirq_irqoff);

void __do_softirq(void)
{
    __u32 pending;
    int softirq_bit;
    struct softirq_action *h;

    pending = local_softirq_pending();

    /* Reset the pending bitmask before enabling irqs */
    set_softirq_pending(0);

    h = softirq_vec;

    while ((softirq_bit = ffs(pending))) {
        h += softirq_bit - 1;
        h->action(h);
        h++;

        pending >>= softirq_bit;
    }
}
EXPORT_SYMBOL(__do_softirq);

int
init_module(void)
{
    printk("module[softirq]: init begin ...\n");
    printk("module[softirq]: init end!\n");
    return 0;
}

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <types.h>
#include <irqreturn.h>

#define IRQ_AFFINITY_MAX_SETS  4

#define IRQF_SHARED 0x00000080

#define local_softirq_pending_ref irq_stat.__softirq_pending

#define local_softirq_pending() local_softirq_pending_ref
#define set_softirq_pending(x)  (local_softirq_pending_ref = (x))
#define or_softirq_pending(x)   (local_softirq_pending_ref |= (x))

enum
{
    HI_SOFTIRQ = 0,
    TIMER_SOFTIRQ,
    NET_TX_SOFTIRQ,
    NET_RX_SOFTIRQ,
    BLOCK_SOFTIRQ,
    IRQ_POLL_SOFTIRQ,
    TASKLET_SOFTIRQ,
    SCHED_SOFTIRQ,
    HRTIMER_SOFTIRQ,
    RCU_SOFTIRQ,

    NR_SOFTIRQS
};

typedef irqreturn_t (*irq_handler_t)(int, void *);

/**
 * struct irq_affinity - Description for automatic irq affinity assignements
 * @pre_vectors:    Don't apply affinity to @pre_vectors at beginning of
 *          the MSI(-X) vector space
 * @post_vectors:   Don't apply affinity to @post_vectors at end of
 *          the MSI(-X) vector space
 * @nr_sets:        The number of interrupt sets for which affinity
 *          spreading is required
 * @set_size:       Array holding the size of each interrupt set
 * @calc_sets:      Callback for calculating the number and size
 *          of interrupt sets
 * @priv:       Private data for usage by @calc_sets, usually a
 *          pointer to driver/device specific data.
 */
struct irq_affinity {
    unsigned int    pre_vectors;
    unsigned int    post_vectors;
    unsigned int    nr_sets;
    unsigned int    set_size[IRQ_AFFINITY_MAX_SETS];
    void            (*calc_sets)(struct irq_affinity *, unsigned int nvecs);
    void            *priv;
};

/**
 * struct irq_affinity_desc - Interrupt affinity descriptor
 * @mask:   cpumask to hold the affinity assignment
 * @is_managed: 1 if the interrupt is managed internally
 */
struct irq_affinity_desc {
};

struct irqaction {
    irq_handler_t handler;
    void *percpu_dev_id;
    struct irqaction *next;
    irq_handler_t thread_fn;
    unsigned int irq;
    unsigned int flags;
    const char *name;
    void *dev_id;
};

struct softirq_action
{
    void (*action)(struct softirq_action *);
};

int
request_threaded_irq(unsigned int irq, irq_handler_t handler,
                     irq_handler_t thread_fn,
                     unsigned long flags, const char *name, void *dev);

static inline int
request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
            const char *name, void *dev)
{
    return request_threaded_irq(irq, handler, NULL, flags, name, dev);
}

int
__irq_set_affinity(unsigned int irq,
                   const struct cpumask *cpumask,
                   bool force);

/**
 * irq_set_affinity - Set the irq affinity of a given irq
 * @irq:    Interrupt to set affinity
 * @cpumask:    cpumask
 *
 * Fails if cpumask does not contain an online CPU
 */
static inline int
irq_set_affinity(unsigned int irq, const struct cpumask *cpumask)
{
    return __irq_set_affinity(irq, cpumask, false);
}

void raise_softirq_irqoff(unsigned int nr);

void __do_softirq(void);

static inline void do_softirq_own_stack(void)
{
    __do_softirq();
}

#endif /* _LINUX_INTERRUPT_H */

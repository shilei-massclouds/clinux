// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/interrupt.h>
#include "../../booter/src/booter.h"

int
cl_rcu_init(void)
{
    sbi_puts("module[rcu]: init begin ...\n");
    sbi_puts("module[rcu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_rcu_init);

void resched_cpu(int cpu)
{
    booter_panic("No impl in 'rcu'.");
}

inline void raise_softirq_irqoff(unsigned int nr)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(raise_softirq_irqoff);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_early_sched_init(void)
{
    sbi_puts("module[early_sched]: init begin ...\n");
    sbi_puts("module[early_sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_sched_init);

#ifndef CONFIG_PREEMPTION
int __sched _cond_resched(void)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("_cond_resched");
    }
    return 0;
}
EXPORT_SYMBOL(_cond_resched);
#endif

#ifdef CONFIG_DEBUG_ATOMIC_SLEEP
void __might_sleep(const char *file, int line, int preempt_offset)
{
    ___might_sleep(file, line, preempt_offset);
}
EXPORT_SYMBOL(__might_sleep);

void ___might_sleep(const char *file, int line, int preempt_offset)
{
    if (system_state != SYSTEM_BOOTING) {
        booter_panic("___might_sleep");
    }
}
EXPORT_SYMBOL(___might_sleep);
#endif

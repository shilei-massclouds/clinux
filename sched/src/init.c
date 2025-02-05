// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

enum system_states system_state __read_mostly;
EXPORT_SYMBOL(system_state);

int
cl_sched_init(void)
{
    sbi_puts("module[sched]: init begin ...\n");
    ENABLE_COMPONENT(base_init);
    sbi_puts("module[sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_sched_init);

/*
void add_taint(unsigned flag, enum lockdep_ok lockdep_ok)
{
    booter_panic("No impl 'add_taint'.");
}
*/

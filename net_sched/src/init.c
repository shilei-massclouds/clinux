// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/sch_generic.h>
#include "../../booter/src/booter.h"

int
cl_net_sched_init(void)
{
    sbi_puts("module[net_sched]: init begin ...\n");
    sbi_puts("module[net_sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_net_sched_init);

struct Qdisc_ops mq_qdisc_ops __read_mostly;

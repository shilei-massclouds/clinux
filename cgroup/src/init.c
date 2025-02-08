// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/cgroup-defs.h>
#include "../../booter/src/booter.h"

int
cl_cgroup_init(void)
{
    sbi_puts("module[cgroup]: init begin ...\n");
    sbi_puts("module[cgroup]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_cgroup_init);

void cgroup1_release_agent(struct work_struct *work)
{
    booter_panic("No impl 'cgroup1_release_agent'.");
}

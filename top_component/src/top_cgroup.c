// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_cgroup_init(void)
{
    sbi_puts("module[top_cgroup]: init begin ...\n");
    REQUIRE_COMPONENT(cgroup);
    cl_init();
    sbi_puts("module[top_cgroup]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_cgroup_init);

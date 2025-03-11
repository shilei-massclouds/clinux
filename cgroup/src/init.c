// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs_parser.h>
#include <linux/cgroup-defs.h>
#include <linux/kernfs.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern int __init cgroup_init_early(void);

int
cl_cgroup_init(void)
{
    sbi_puts("module[cgroup]: init begin ...\n");
    cgroup_init_early();
    sbi_puts("module[cgroup]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_cgroup_init);

DEFINE_ENABLE_FUNC(cgroup);

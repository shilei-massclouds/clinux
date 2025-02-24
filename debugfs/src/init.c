// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_debugfs_init(void)
{
    sbi_puts("module[debugfs]: init begin ...\n");
    REQUIRE_COMPONENT(ksysfs);
    sbi_puts("module[debugfs]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_debugfs_init);

DEFINE_ENABLE_FUNC(debugfs);

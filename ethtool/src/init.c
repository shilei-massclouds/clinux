// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_ethtool_init(void)
{
    sbi_puts("module[ethtool]: init begin ...\n");
    sbi_puts("module[ethtool]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ethtool_init);

DEFINE_ENABLE_FUNC(ethtool);

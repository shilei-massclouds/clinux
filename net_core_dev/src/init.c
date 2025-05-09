// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_net_core_dev_init(void)
{
    sbi_puts("module[net_core_dev]: init begin ...\n");
    sbi_puts("module[net_core_dev]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_net_core_dev_init);

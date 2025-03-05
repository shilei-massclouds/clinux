// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include "../../booter/src/booter.h"

int
cl_devinet_init(void)
{
    sbi_puts("module[devinet]: init begin ...\n");
    sbi_puts("module[devinet]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_devinet_init);

int sysctl_devconf_inherit_init_net __read_mostly;

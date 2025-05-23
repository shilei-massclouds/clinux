// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/jump_label.h>
#include <linux/cache.h>
#include <linux/fs.h>
#include <linux/splice.h>
#include <cl_hook.h>
#include <net/dst.h>
#include <net/netlink.h>
#include "../../booter/src/booter.h"

int
cl_net_core_init(void)
{
    sbi_puts("module[net_core]: init begin ...\n");
    sbi_puts("module[net_core]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_net_core_init);

DEFINE_ENABLE_FUNC(net_core);

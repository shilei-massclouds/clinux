// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/wait.h>
#include <net/netlink.h>
#include <net/dst.h>
#include "../../booter/src/booter.h"

int
cl_netlink_init(void)
{
    sbi_puts("module[netlink]: init begin ...\n");
    sbi_puts("module[netlink]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_netlink_init);

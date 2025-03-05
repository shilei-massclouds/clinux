// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include "../../booter/src/booter.h"

int
cl_tcp_metrics_init(void)
{
    sbi_puts("module[tcp_metrics]: init begin ...\n");
    sbi_puts("module[tcp_metrics]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tcp_metrics_init);

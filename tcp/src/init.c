// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/tcp.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_tcp_init(void)
{
    sbi_puts("module[tcp]: init begin ...\n");
    sbi_puts("module[tcp]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tcp_init);

DEFINE_ENABLE_FUNC(tcp);

struct tcp_congestion_ops tcp_reno;
EXPORT_SYMBOL(tcp_reno);

int sysctl_tcp_max_orphans __read_mostly = NR_FILE;

struct inet_hashinfo tcp_hashinfo;
EXPORT_SYMBOL(tcp_hashinfo);

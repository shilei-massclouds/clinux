// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/tcp.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_tcp_ipv4_init(void)
{
    sbi_puts("module[tcp_ipv4]: init begin ...\n");
    sbi_puts("module[tcp_ipv4]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tcp_ipv4_init);

DEFINE_ENABLE_FUNC(tcp_ipv4);

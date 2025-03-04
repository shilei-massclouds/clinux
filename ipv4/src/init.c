// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/ping.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_ipv4_init(void)
{
    sbi_puts("module[ipv4]: init begin ...\n");
    sbi_puts("module[ipv4]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipv4_init);

DEFINE_ENABLE_FUNC(ipv4);

struct proto raw_prot;
struct proto ping_prot;
struct proto udp_prot;
struct pingv6_ops pingv6_ops;
EXPORT_SYMBOL_GPL(pingv6_ops);

const struct net_offload __rcu *inet_offloads[MAX_INET_PROTOS] __read_mostly;
EXPORT_SYMBOL(inet_offloads);

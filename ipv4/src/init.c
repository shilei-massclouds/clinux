// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/ping.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"
#include "../fib_lookup.h"

int
cl_ipv4_init(void)
{
    sbi_puts("module[ipv4]: init begin ...\n");
    sbi_puts("module[ipv4]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipv4_init);

DEFINE_ENABLE_FUNC(ipv4);

struct proto ping_prot;
struct pingv6_ops pingv6_ops;
EXPORT_SYMBOL_GPL(pingv6_ops);

const struct net_offload __rcu *inet_offloads[MAX_INET_PROTOS] __read_mostly;
EXPORT_SYMBOL(inet_offloads);

struct net_device *blackhole_netdev;
EXPORT_SYMBOL(blackhole_netdev);

struct neigh_table arp_tbl;
const struct fib_prop fib_props[RTN_MAX + 1];
const struct nla_policy rtm_ipv4_policy[RTA_MAX + 1];

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

DEFINE_STATIC_KEY_FALSE(memalloc_socks_key);
EXPORT_SYMBOL_GPL(memalloc_socks_key);

int sysctl_tstamp_allow_data __read_mostly = 1;

struct pipe_buf_operations;
const struct pipe_buf_operations nosteal_pipe_buf_ops;
const struct nla_policy nda_policy[1];
const struct dst_metrics dst_default_metrics;
DEFINE_STATIC_KEY_FALSE(bpf_stats_enabled_key);
struct pernet_operations __net_initdata loopback_net_ops;

atomic_t genl_sk_destructing_cnt = ATOMIC_INIT(0);
DECLARE_WAIT_QUEUE_HEAD(genl_sk_destructing_waitq);

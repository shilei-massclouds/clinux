// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/jump_label.h>
#include <net/ip.h>
#include <net/ip_tunnels.h>
#include <net/icmp.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_udp_init(void)
{
    sbi_puts("module[udp]: init begin ...\n");
    sbi_puts("module[udp]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_udp_init);

DEFINE_ENABLE_FUNC(udp);

DEFINE_STATIC_KEY_FALSE(bpf_sk_lookup_enabled);
EXPORT_SYMBOL(bpf_sk_lookup_enabled);

const struct ip_tunnel_encap_ops __rcu *
        iptun_encaps[MAX_IPTUN_ENCAP_OPS] __read_mostly;
EXPORT_SYMBOL(iptun_encaps);


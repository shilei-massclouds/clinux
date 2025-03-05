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

void *genlmsg_put(struct sk_buff *skb, u32 portid, u32 seq,
          const struct genl_family *family, int flags, u8 cmd)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(genlmsg_put);

int genl_register_family(struct genl_family *family)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(genl_register_family);


// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <net/flow_dissector.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_virtio_net_init(void)
{
    sbi_puts("module[virtio_net]: init begin ...\n");
    sbi_puts("module[virtio_net]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_virtio_net_init);

DEFINE_ENABLE_FUNC(virtio_net);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/netdevice.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_link_watch_init(void)
{
    sbi_puts("module[link_watch]: init begin ...\n");
    sbi_puts("module[link_watch]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_link_watch_init);

DEFINE_ENABLE_FUNC(link_watch);

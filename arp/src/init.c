// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_arp_init(void)
{
    sbi_puts("module[arp]: init begin ...\n");
    sbi_puts("module[arp]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_arp_init);

DEFINE_ENABLE_FUNC(arp);

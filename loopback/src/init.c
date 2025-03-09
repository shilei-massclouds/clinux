// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_loopback_init(void)
{
    sbi_puts("module[loopback]: init begin ...\n");
    sbi_puts("module[loopback]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_loopback_init);

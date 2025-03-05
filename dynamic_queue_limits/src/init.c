// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_dynamic_queue_limits_init(void)
{
    sbi_puts("module[dynamic_queue_limits]: init begin ...\n");
    sbi_puts("module[dynamic_queue_limits]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dynamic_queue_limits_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include "../../booter/src/booter.h"

int
cl_aio_init(void)
{
    sbi_puts("module[aio]: init begin ...\n");
    sbi_puts("module[aio]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_aio_init);

DEFINE_PER_CPU(int, eventfd_wake_count);

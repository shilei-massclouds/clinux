// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_cpu_init(void)
{
    sbi_puts("module[cpu]: init begin ...\n");
    sbi_puts("module[cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_cpu_init);

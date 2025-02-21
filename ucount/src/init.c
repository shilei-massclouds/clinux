// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <vdso/limits.h>
#include "../../booter/src/booter.h"

int
cl_ucount_init(void)
{
    sbi_puts("module[ucount]: init begin ...\n");
    sbi_puts("module[ucount]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ucount_init);

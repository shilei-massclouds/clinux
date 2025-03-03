// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_mprotect_init(void)
{
    sbi_puts("module[mprotect]: init begin ...\n");
    sbi_puts("module[mprotect]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_mprotect_init);

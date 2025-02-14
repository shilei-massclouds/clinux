// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_module_init(void)
{
    sbi_puts("module[module]: init begin ...\n");
    sbi_puts("module[module]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_module_init);

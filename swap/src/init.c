// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/swap.h>
#include "../../booter/src/booter.h"

int
cl_swap_init(void)
{
    sbi_puts("module[swap]: init begin ...\n");
    sbi_puts("module[swap]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_swap_init);

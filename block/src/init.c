// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include <linux/kobject.h>
#include <linux/bio.h>
#include "../../booter/src/booter.h"

int
cl_block_init(void)
{
    sbi_puts("module[block]: init begin ...\n");
    sbi_puts("module[block]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_block_init);

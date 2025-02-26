// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include "../../booter/src/booter.h"

int
cl_bio_init(void)
{
    sbi_puts("module[bio]: init begin ...\n");
    sbi_puts("module[bio]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_bio_init);

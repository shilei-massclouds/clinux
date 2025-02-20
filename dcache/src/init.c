// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_dcache_init(void)
{
    sbi_puts("module[dcache]: init begin ...\n");
    sbi_puts("module[dcache]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dcache_init);

__weak void __init mnt_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(mnt_init);

__weak void __init bdev_cache_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(bdev_cache_init);

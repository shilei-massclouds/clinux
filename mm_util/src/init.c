// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_mm_util_init(void)
{
    sbi_puts("module[mm_util]: init begin ...\n");
    sbi_puts("module[mm_util]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_mm_util_init);

void* __weak __kmalloc_track_caller(size_t size, gfp_t flags, unsigned long caller)
{
    booter_panic("No impl 'mm_util'.");
}
EXPORT_SYMBOL(__kmalloc_track_caller);

void __weak kfree(const void *objp)
{
    booter_panic("No impl 'mm_util'.");
}
EXPORT_SYMBOL(kfree);

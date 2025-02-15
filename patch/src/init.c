// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_patch_init(void)
{
    sbi_puts("module[patch]: init begin ...\n");
    sbi_puts("module[patch]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_patch_init);

/*
long copy_to_kernel_nofault(void *dst, const void *src, size_t size)
{
    booter_panic("No impl!\n");
}
*/


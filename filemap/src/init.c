// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_filemap_init(void)
{
    sbi_puts("module[filemap]: init begin ...\n");
    sbi_puts("module[filemap]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_filemap_init);

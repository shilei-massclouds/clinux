// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_readdir_init(void)
{
    sbi_puts("module[readdir]: init begin ...\n");
    sbi_puts("module[readdir]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_readdir_init);

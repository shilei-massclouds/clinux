// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/seq_file.h>
#include "../../booter/src/booter.h"

int
cl_filesystems_init(void)
{
    sbi_puts("module[filesystems]: init begin ...\n");
    sbi_puts("module[filesystems]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_filesystems_init);

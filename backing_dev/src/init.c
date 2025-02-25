// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/flex_proportions.h>
#include "../../booter/src/booter.h"

int
cl_backing_dev_init(void)
{
    sbi_puts("module[backing_dev]: init begin ...\n");
    sbi_puts("module[backing_dev]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_backing_dev_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_block_dev_init(void)
{
    sbi_puts("module[block_dev]: init begin ...\n");
    sbi_puts("module[block_dev]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_block_dev_init);

DEFINE_ENABLE_FUNC(block_dev);

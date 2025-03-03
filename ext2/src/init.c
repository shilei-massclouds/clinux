// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/highuid.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_ext2_init(void)
{
    sbi_puts("module[ext2]: init begin ...\n");
    sbi_puts("module[ext2]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ext2_init);

DEFINE_ENABLE_FUNC(ext2);

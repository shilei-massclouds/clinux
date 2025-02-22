// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_devtmpfs_init(void)
{
    sbi_puts("module[devtmpfs]: init begin ...\n");
    sbi_puts("module[devtmpfs]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_devtmpfs_init);

DEFINE_ENABLE_FUNC(devtmpfs);

struct class block_class;

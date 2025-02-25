// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_fs_writeback_init(void)
{
    sbi_puts("module[fs_writeback]: init begin ...\n");
    sbi_puts("module[fs_writeback]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_fs_writeback_init);

DEFINE_ENABLE_FUNC(fs_writeback);

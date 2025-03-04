// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/msg.h>
#include "../../booter/src/booter.h"
#include "../util.h"

int
cl_ipc_init(void)
{
    sbi_puts("module[ipc]: init begin ...\n");
    sbi_puts("module[ipc]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipc_init);

int ipc_mni_shift = IPCMNI_SHIFT;
int ipc_min_cycle = RADIX_TREE_MAP_SIZE;

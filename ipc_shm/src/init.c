// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/msg.h>
#include "../ipc/util.h"
#include "../../booter/src/booter.h"

int
cl_ipc_shm_init(void)
{
    sbi_puts("module[ipc_shm]: init begin ...\n");
    sbi_puts("module[ipc_shm]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipc_shm_init);

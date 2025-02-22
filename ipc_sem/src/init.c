// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/msg.h>
#include "../ipc/util.h"
#include "../../booter/src/booter.h"

int
cl_ipc_sem_init(void)
{
    sbi_puts("module[ipc_sem]: init begin ...\n");
    sbi_puts("module[ipc_sem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ipc_sem_init);

int ipc_mni = IPCMNI;

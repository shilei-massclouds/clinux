// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/srcu.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_notify_init(void)
{
    sbi_puts("module[notify]: init begin ...\n");
    sbi_puts("module[notify]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_notify_init);

DEFINE_ENABLE_FUNC(notify);

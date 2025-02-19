// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_vmscan_init(void)
{
    sbi_puts("module[vmscan]: init begin ...\n");
    sbi_puts("module[vmscan]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_vmscan_init);

DEFINE_ENABLE_FUNC(vmscan);

int laptop_mode;
EXPORT_SYMBOL(laptop_mode);

int buffer_heads_over_limit;


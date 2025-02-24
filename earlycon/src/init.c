// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/console.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_earlycon_init(void)
{
    sbi_puts("module[earlycon]: init begin ...\n");
    sbi_puts("module[earlycon]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_earlycon_init);

DEFINE_ENABLE_FUNC(earlycon);

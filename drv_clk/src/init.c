// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_drv_clk_init(void)
{
    sbi_puts("module[drv_clk]: init begin ...\n");
    sbi_puts("module[drv_clk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_drv_clk_init);

DEFINE_ENABLE_FUNC(drv_clk);

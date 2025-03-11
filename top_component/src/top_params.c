// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include <linux/string.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_params_init(void)
{
    sbi_puts("module[top_params]: init begin ...\n");
    ENABLE_COMPONENT(early_printk);
    ENABLE_COMPONENT(params);
    cl_init();
    sbi_puts("module[top_params]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_params_init);

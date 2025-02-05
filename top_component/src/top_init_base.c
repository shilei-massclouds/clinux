// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_init_base_init(void)
{
    sbi_puts("module[top_init_base]: init begin ...\n");
    ENABLE_COMPONENT(init_base);
    sbi_puts("module[top_init_base]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_init_base_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_bootmem_init(void)
{
    sbi_puts("module[top_bootmem]: init begin ...\n");
    ENABLE_COMPONENT(early_printk);
    parse_dtb();
    setup_kernel_in_mm();
    parse_early_param();
    setup_bootmem();
    sbi_puts("module[top_bootmem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_bootmem_init);

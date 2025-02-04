// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <asm/pgtable.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_paging_init(void)
{
    sbi_puts("module[top_paging]: init begin ...\n");
    ENABLE_COMPONENT(early_printk);
    parse_dtb();
    setup_kernel_in_mm();
    parse_early_param();
    setup_bootmem();
    paging_init();
    sbi_puts("module[top_paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_paging_init);

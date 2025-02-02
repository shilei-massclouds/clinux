// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/of_fdt.h>
#include "../../booter/src/booter.h"

extern void *dtb_early_va;

static void parse_dtb(void)
{
    if (early_init_dt_scan(dtb_early_va))
        return;

    booter_panic("No DTB passed to the kernel\n");
}

int
cl_top_early_fdt_init(void)
{
    sbi_puts("module[top_early_fdt]: init begin ...\n");
    parse_dtb();
    sbi_puts("module[top_early_fdt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_early_fdt_init);

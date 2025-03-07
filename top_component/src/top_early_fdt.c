// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/memblock.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_early_fdt_init(void)
{
    sbi_puts("module[top_early_fdt]: init begin ...\n");

    ENABLE_COMPONENT(early_printk);

    parse_dtb();

    sbi_puts("BOOT Command Line:\n ");
    sbi_puts(boot_command_line);
    sbi_puts("\n");

    // Enable early-option 'memblock'='debug' to dump memblock.
    do_early_param("memblock", "debug", NULL, NULL);
    memblock_dump_all();

    sbi_puts("module[top_early_fdt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_early_fdt_init);

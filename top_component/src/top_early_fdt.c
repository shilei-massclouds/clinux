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
    cl_init();

    printk("boot command line: [%s]\n", boot_command_line);

    // Enable early-option 'memblock'='debug' to dump memblock.
    do_early_param("memblock", "debug", NULL, NULL);
    memblock_dump_all();

    sbi_puts("module[top_early_fdt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_early_fdt_init);

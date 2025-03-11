// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern int memblock_debug;

int
cl_top_bootmem_init(void)
{
    sbi_puts("module[top_bootmem]: init begin ...\n");
    REQUIRE_COMPONENT(bootmem);
    REQUIRE_COMPONENT(early_printk);

    /* Enable memblock to dump info. */
    memblock_debug = 1;
    cl_init();

    sbi_puts("module[top_bootmem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_bootmem_init);

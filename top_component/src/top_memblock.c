// SPDX-License-Identifier: GPL-2.0-only

#include <linux/memblock.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_memblock_init(void)
{
    int ret;
    sbi_puts("module[top_memblock]: init begin ...\n");
    REQUIRE_COMPONENT(memblock);
    cl_init();
    sbi_puts("module[top_memblock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_memblock_init);

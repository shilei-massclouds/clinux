// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_dma_init(void)
{
    sbi_puts("module[top_dma]: init begin ...\n");
    REQUIRE_COMPONENT(dma);
    cl_init();
    sbi_puts("module[top_dma]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_dma_init);

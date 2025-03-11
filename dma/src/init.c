// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/swiotlb.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_dma_init(void)
{
    sbi_puts("module[dma]: init begin ...\n");
    swiotlb_init(1);
    sbi_puts("module[dma]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dma_init);

DEFINE_ENABLE_FUNC(dma);

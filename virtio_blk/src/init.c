// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_virtio_blk_init(void)
{
    sbi_puts("module[virtio_blk]: init begin ...\n");
    sbi_puts("module[virtio_blk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_virtio_blk_init);

DEFINE_ENABLE_FUNC(virtio_blk);

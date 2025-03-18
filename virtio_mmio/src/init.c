// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern int virtio_mmio_init(void);

int
cl_virtio_mmio_init(void)
{
    sbi_puts("module[virtio_mmio]: init begin ...\n");
    virtio_mmio_init();
    sbi_puts("module[virtio_mmio]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_virtio_mmio_init);

DEFINE_ENABLE_FUNC(virtio_mmio);


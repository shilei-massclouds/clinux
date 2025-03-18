// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

extern int virtio_init(void);

int
cl_virtio_init(void)
{
    sbi_puts("module[virtio]: init begin ...\n");
    virtio_init();
    sbi_puts("module[virtio]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_virtio_init);

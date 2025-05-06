// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/printk.h>
#include "../../booter/src/booter.h"

extern int virtio_blk_init(void);

int
cl_top_virtio_blk_init(void)
{
    int ret;
    sbi_puts("module[top_virtio_blk]: init begin ...\n");

    ret = virtio_blk_init();
    if (ret < 0) {
        printk("Init virtio_blk error[%d]!\n", ret);
        booter_panic("Init virtio_blk error!");
    }

    sbi_puts("module[top_virtio_blk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_virtio_blk_init);

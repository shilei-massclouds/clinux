// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/genhd.h>
#include <linux/virtio.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

static int major;
static struct workqueue_struct *virtblk_wq;
extern struct virtio_driver virtio_blk;

int
cl_top_virtio_blk_init(void)
{
	int error;

    sbi_puts("module[top_virtio_blk]: init begin ...\n");
    REQUIRE_COMPONENT(virtio_blk);
    REQUIRE_COMPONENT(early_printk);
    cl_init();

	virtblk_wq = alloc_workqueue("virtio-blk", 0, 0);
	if (!virtblk_wq)
		return -ENOMEM;

	major = register_blkdev(0, "virtblk");
	if (major < 0) {
		return major;
	}

	error = register_virtio_driver(&virtio_blk);
	if (error) {
		return major;
    }

    sbi_puts("module[top_virtio_blk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_virtio_blk_init);

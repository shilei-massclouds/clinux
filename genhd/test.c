// SPDX-License-Identifier: GPL-2.0-only

#include <genhd.h>
#include <printk.h>

static int
test_register_blkdev(void)
{
    int major;
    major = register_blkdev(0, "virtblk");
    if (major <= 0)
        return -1;

    return 0;
}

static int
init_module(void)
{
    printk("module[test_genhd]: init begin ...\n");

    if (test_register_blkdev()) {
        printk(_RED("test register blkdev failed!\n"));
        return -1;
    }

    printk("module[test_genhd]: init end!\n");
}

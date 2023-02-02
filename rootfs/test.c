// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <bug.h>
#include <stat.h>
#include <mount.h>
#include <printk.h>

static int
test_create_dir(void)
{
    umode_t mode;
    mode = S_IFDIR | S_IRUGO | S_IWUSR | S_IXUGO;
    return init_mkdir("dev", mode);
}

static int
test_create_dev(void)
{
    return create_dev("/dev/root", 0xFE00001);
}

static int
init_module(void)
{
    printk("module[test_rootfs]: init begin ...\n");

    BUG_ON(!rootfs_initialized);

    if (test_create_dir()) {
        printk(_RED("test dir create failed!\n"));
        return -1;
    }

    if (test_create_dev()) {
        printk(_RED("test dev create failed!\n"));
        return -1;
    }

    printk("module[test_rootfs]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <export.h>
#include <printk.h>

extern int init_ext2_fs(void);

bool ext2_initialized = false;
EXPORT_SYMBOL(ext2_initialized);

int
init_module(void)
{
    printk("module[ext2]: init begin ...\n");

    init_ext2_fs();
    ext2_initialized = true;

    printk("module[ext2]: init end!\n");
    return 0;
}

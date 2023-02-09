// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <export.h>
#include <printk.h>

bool ext2_initialized = false;
EXPORT_SYMBOL(ext2_initialized);

static int
init_module(void)
{
    printk("module[rs_ext2]: init begin ...\n");

    ext2_initialized = true;

    printk("module[rs_ext2]: init end!\n");
    return 0;
}

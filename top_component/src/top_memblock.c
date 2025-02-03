// SPDX-License-Identifier: GPL-2.0-only

#include <linux/memblock.h>
#include "../../booter/src/booter.h"

int
cl_top_memblock_init(void)
{
    int ret;
    sbi_puts("module[top_memblock]: init begin ...\n");
    ret = memblock_add(0, 1024);
    printk("memblock_add: ret (%d)\n", ret);
    sbi_puts("module[top_memblock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_memblock_init);

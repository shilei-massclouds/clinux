// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/module.h>
#include "../../booter/src/booter.h"

static int __init init_lib(void)
{
    sbi_puts("module[lib]: init begin ...\n");
    sbi_puts("module[lib]: init end!\n");
    return 0;
}

module_init(init_lib);

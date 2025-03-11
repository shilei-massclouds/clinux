// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern void __init do_initcalls(void);

int
cl_initcalls_init(void)
{
    sbi_puts("module[initcalls]: init begin ...\n");
    do_initcalls();
    sbi_puts("module[initcalls]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_initcalls_init);

DEFINE_ENABLE_FUNC(initcalls);

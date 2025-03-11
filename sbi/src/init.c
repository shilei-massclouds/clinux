// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern int __init sbi_init(void);

int
cl_sbi_init(void)
{
    sbi_puts("module[sbi]: init begin ...\n");
    sbi_init();
    sbi_puts("module[sbi]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_sbi_init);

DEFINE_ENABLE_FUNC(sbi);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern void __init boot_cpu_init(void);

int
cl_cpu_init(void)
{
    sbi_puts("module[cpu]: init begin ...\n");
    boot_cpu_init();
    sbi_puts("module[cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_cpu_init);

DEFINE_ENABLE_FUNC(cpu);

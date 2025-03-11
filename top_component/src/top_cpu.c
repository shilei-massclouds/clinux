// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cpumask.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_cpu_init(void)
{
    sbi_puts("module[top_cpu]: init begin ...\n");
    REQUIRE_COMPONENT(cpu);
    REQUIRE_COMPONENT(early_printk);
    cl_init();

    printk("cpu_all_bits: [%lx]\n", cpu_all_bits[0]);
    sbi_puts("module[top_cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_cpu_init);

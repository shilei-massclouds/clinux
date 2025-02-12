// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_riscv_cpu_init(void)
{
    sbi_puts("module[riscv_cpu]: init begin ...\n");
    sbi_puts("module[riscv_cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_riscv_cpu_init);

void __init __weak smp_setup_processor_id(void)
{
}
EXPORT_SYMBOL(smp_setup_processor_id);

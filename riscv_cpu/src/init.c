// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern void riscv_fill_hwcap(void);

int
cl_riscv_cpu_init(void)
{
    sbi_puts("module[riscv_cpu]: init begin ...\n");
    REQUIRE_COMPONENT(of);
    riscv_fill_hwcap();
    sbi_puts("module[riscv_cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_riscv_cpu_init);

DEFINE_ENABLE_FUNC(riscv_cpu);

void __init __weak smp_setup_processor_id(void)
{
}
EXPORT_SYMBOL(smp_setup_processor_id);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_riscv_cpu_init(void)
{
    sbi_puts("module[top_riscv_cpu]: init begin ...\n");
    REQUIRE_COMPONENT(riscv_cpu);
    cl_init();
    sbi_puts("module[top_riscv_cpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_riscv_cpu_init);

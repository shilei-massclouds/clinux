// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_riscv_mm_context_init(void)
{
    sbi_puts("module[riscv_mm_context]: init begin ...\n");
    sbi_puts("module[riscv_mm_context]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_riscv_mm_context_init);

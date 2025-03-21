// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_irq_sifive_plic_init(void)
{
    sbi_puts("module[irq_sifive_plic]: init begin ...\n");
    sbi_puts("module[irq_sifive_plic]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_irq_sifive_plic_init);

DEFINE_ENABLE_FUNC(irq_sifive_plic);

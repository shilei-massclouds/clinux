// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_of_irq_init(void)
{
    sbi_puts("module[of_irq]: init begin ...\n");
    sbi_puts("module[of_irq]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_of_irq_init);

DEFINE_ENABLE_FUNC(of_irq);

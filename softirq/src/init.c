// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_softirq_init(void)
{
    sbi_puts("module[softirq]: init begin ...\n");
    sbi_puts("module[softirq]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_softirq_init);

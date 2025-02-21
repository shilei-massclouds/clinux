// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_percpu_refcount_init(void)
{
    sbi_puts("module[percpu_refcount]: init begin ...\n");
    sbi_puts("module[percpu_refcount]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_percpu_refcount_init);

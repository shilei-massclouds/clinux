// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_top_booter_init(void)
{
    sbi_puts("module[verify_booter]: init begin ...\n");
    sbi_puts("Booter: Hello, New World!\n");
    sbi_puts("module[verify_booter]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_booter_init);

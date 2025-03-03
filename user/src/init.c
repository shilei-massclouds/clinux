// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_user_init(void)
{
    sbi_puts("module[user]: init begin ...\n");
    sbi_puts("module[user]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_user_init);

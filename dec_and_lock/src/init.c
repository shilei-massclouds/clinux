// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_dec_and_lock_init(void)
{
    sbi_puts("module[dec_and_lock]: init begin ...\n");
    sbi_puts("module[dec_and_lock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dec_and_lock_init);

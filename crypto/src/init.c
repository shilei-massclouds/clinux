// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_crypto_init(void)
{
    sbi_puts("module[crypto]: init begin ...\n");
    sbi_puts("module[crypto]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_crypto_init);

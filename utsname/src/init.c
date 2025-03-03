// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/user_namespace.h>
#include "../../booter/src/booter.h"

int
cl_utsname_init(void)
{
    sbi_puts("module[utsname]: init begin ...\n");
    sbi_puts("module[utsname]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_utsname_init);

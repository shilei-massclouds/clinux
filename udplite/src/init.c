// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_udplite_init(void)
{
    sbi_puts("module[udplite]: init begin ...\n");
    sbi_puts("module[udplite]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_udplite_init);

DEFINE_ENABLE_FUNC(udplite);

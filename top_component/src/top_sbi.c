// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_sbi_init(void)
{
    sbi_puts("module[top_sbi]: init begin ...\n");
    REQUIRE_COMPONENT(sbi);
    cl_init();
    sbi_puts("module[top_sbi]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_sbi_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

void test(void)
{
    // Implement test code here.
}

int
cl_top_XXX_init(void)
{
    sbi_puts("module[top_XXX]: init begin ...\n");

    cl_init();
    test();

    sbi_puts("module[top_XXX]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_XXX_init);

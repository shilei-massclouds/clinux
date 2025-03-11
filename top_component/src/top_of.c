// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/of.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_of_init(void)
{
    sbi_puts("module[top_of]: init begin ...\n");
    REQUIRE_COMPONENT(of);
    REQUIRE_COMPONENT(early_printk);
    cl_init();
    if (!of_have_populated_dt()) {
        booter_panic("of_root is NULL!\n");
    }
    sbi_puts("module[top_of]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_of_init);

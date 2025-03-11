// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_resource_init(void)
{
    sbi_puts("module[top_resource]: init begin ...\n");
    REQUIRE_COMPONENT(resource);
    cl_init();
    sbi_puts("module[top_resource]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_resource_init);

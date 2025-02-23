// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/capability.h>
#include <linux/cred.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_capability_init(void)
{
    sbi_puts("module[capability]: init begin ...\n");
    sbi_puts("module[capability]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_capability_init);

DEFINE_ENABLE_FUNC(capability);

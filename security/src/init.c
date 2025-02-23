// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_security_init(void)
{
    sbi_puts("module[security]: init begin ...\n");
    sbi_puts("module[security]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_security_init);

DEFINE_ENABLE_FUNC(security);

unsigned long dac_mmap_min_addr = CONFIG_DEFAULT_MMAP_MIN_ADDR;

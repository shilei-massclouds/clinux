// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sysfs.h>
#include "../../booter/src/booter.h"

int
cl_input_init(void)
{
    sbi_puts("module[input]: init begin ...\n");
    sbi_puts("module[input]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_input_init);

struct attribute_group input_poller_attribute_group;

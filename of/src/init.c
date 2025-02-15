// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include "../../booter/src/booter.h"

int
cl_of_init(void)
{
    sbi_puts("module[of]: init begin ...\n");
    sbi_puts("module[of]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_of_init);

struct device_link *device_link_add(struct device *consumer,
                    struct device *supplier, u32 flags)
{
    booter_panic("No impl in 'of_fdt'.");
}


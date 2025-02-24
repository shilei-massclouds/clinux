// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/console_struct.h>
#include "../../booter/src/booter.h"

int
cl_dummycon_init(void)
{
    sbi_puts("module[dummycon]: init begin ...\n");
    sbi_puts("module[dummycon]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dummycon_init);

__weak int vc_resize(struct vc_data *vc, unsigned int cols, unsigned int rows)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(vc_resize);

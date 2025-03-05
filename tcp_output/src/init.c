// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_tcp_output_init(void)
{
    sbi_puts("module[tcp_output]: init begin ...\n");
    sbi_puts("module[tcp_output]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tcp_output_init);

DEFINE_ENABLE_FUNC(tcp_output);

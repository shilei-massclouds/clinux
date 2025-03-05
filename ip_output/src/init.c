// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include <net/sock.h>
#include "../../booter/src/booter.h"

int
cl_ip_output_init(void)
{
    sbi_puts("module[ip_output]: init begin ...\n");
    sbi_puts("module[ip_output]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ip_output_init);

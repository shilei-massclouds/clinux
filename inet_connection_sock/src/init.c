// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_inet_connection_sock_init(void)
{
    sbi_puts("module[inet_connection_sock]: init begin ...\n");
    sbi_puts("module[inet_connection_sock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_inet_connection_sock_init);

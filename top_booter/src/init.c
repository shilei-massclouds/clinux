// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include "../../booter/src/booter.h"

int
init_module(void)
{
    sbi_puts("module[verify_booter]: init begin ...\n");
    sbi_puts("Booter: Hello, New World!\n");
    sbi_puts("module[verify_booter]: init end!\n");
    return 0;
}

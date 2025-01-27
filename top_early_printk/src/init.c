// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include "../../booter/src/booter.h"

int
init_module(void)
{
    char *s = "Hello, early_printk!";
    int num = 101;
    sbi_puts("module[top_early_printk]: init begin ...\n");
    early_printk("[early_printk]: '%s' '%d' '0x%x'\n", s, num, num);
    sbi_puts("module[top_early_printk]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include "../../booter/src/booter.h"

int
cl_top_printk_init(void)
{
    char *s = "Hello, printk!";
    int num = 101;
    sbi_puts("module[top_printk]: init begin ...\n");
    printk("[printk]: '%s' '%d' '0x%x'\n", s, num, num);
    sbi_puts("module[top_printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_printk_init);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_early_printk_init(void)
{
    char *s = "Hello, early_printk!";
    int num = 101;
    sbi_puts("module[top_early_printk]: init begin ...\n");
    ENABLE_COMPONENT(early_printk);
    cl_init();
    printk("[early_printk]: '%s' '%d' '0x%x'\n", s, num, num);
    sbi_puts("module[top_early_printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_early_printk_init);

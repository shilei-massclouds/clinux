// SPDX-License-Identifier: GPL-2.0-only

#include <linux/printk.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

void test(void)
{
    char *s = "Hello, early_printk!";
    int num = 101;
    printk("[early_printk]: '%s' '%d' '0x%x'\n", s, num, num);
}

int
cl_top_early_printk_init(void)
{
    ENABLE_COMPONENT(early_printk);

    sbi_puts("module[top_early_printk]: init begin ...\n");
    cl_init();
    test();
    sbi_puts("module[top_early_printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_early_printk_init);

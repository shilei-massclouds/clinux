// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

void test(void)
{
    char buf[16];
    memset(buf, 0, sizeof(buf));
    num_to_str(buf, sizeof(buf), 12345, 8);
    sbi_puts("Convert number '12345' to string \"");
    sbi_puts(buf);
    sbi_puts("\"\n");
}

int
cl_top_vsprintf_init(void)
{
    sbi_puts("module[top_vsprintf]: init begin ...\n");

    cl_init();
    test();

    sbi_puts("module[top_vsprintf]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_vsprintf_init);

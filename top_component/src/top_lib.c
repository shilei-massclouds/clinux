// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/string.h>
#include <linux/bitops.h>
#include "../../booter/src/booter.h"

int
cl_top_lib_init(void)
{
    unsigned long data = 2;
    sbi_puts("module[top_lib]: init begin ...\n");
    //int pos = find_last_bit(&data, 16);
    //sbi_put_dec(pos);
    char *p = strchrnul("hello", 'e');
    sbi_put_u64((unsigned long)p);
    memset(&data, 0, sizeof(data));
    sbi_puts("\n");
    sbi_puts("module[top_lib]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_lib_init);

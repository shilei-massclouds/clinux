// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>

extern void say_hello();

int
init_module(void)
{
    sbi_puts("module[helloworld]: init begin ...\n");
    say_hello();
    sbi_puts("module[helloworld]: init end!\n");
    return 0;
}

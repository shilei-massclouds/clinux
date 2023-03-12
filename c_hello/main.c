// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <export.h>

bool
IBase_ready(void)
{
    return true;
}
EXPORT_SYMBOL(IBase_ready);

const char *
IBase_name(void)
{
    return "C_hello";
}
EXPORT_SYMBOL(IBase_name);

int
init_module(void)
{
    printk("module[c_hello]: init begin ...\n");
    printk("C: Hello world!\n");
    printk("module[c_hello]: init end!\n");
    return 0;
}

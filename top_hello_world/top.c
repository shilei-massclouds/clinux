// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool IBase_ready();
extern const char* IBase_name();

int
init_module(void)
{
    printk("module[top_hello_world]: init begin ...\n");
    BUG_ON(!IBase_ready());
    printk("Component '%s'\n", IBase_name());
    printk("module[top_hello_world]: init end!\n");
    return 0;
}

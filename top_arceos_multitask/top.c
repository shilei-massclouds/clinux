// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool libax_ready(void);
extern void multitask(void);

int
init_module(void)
{
    printk("module[top_multitask]: init begin ...\n");
    BUG_ON(!libax_ready());
    multitask();
    printk("module[top_multitask]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <export.h>
#include <printk.h>

extern void rust_main();

bool
axruntime_ready(void)
{
    return true;
}
EXPORT_SYMBOL(axruntime_ready);

int
init_module(void)
{
    printk("module[axruntime]: init begin ...\n");
    BUG_ON(!slab_is_available());
    rust_main();
    printk("module[axruntime]: init end!\n");
    return 0;
}
// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>

extern void rust_main();

/*
#include <export.h>

#define EXPORT_SYMBOL_RUST_GPL(sym) extern int sym; EXPORT_SYMBOL(sym)

#include "exports_axruntime_generated.h"
*/
/*
bool
axruntime_ready(void)
{
    return true;
}
EXPORT_SYMBOL(axruntime_ready);
*/

int
init_module(void)
{
    //printk("module[axruntime]: init begin ...\n");
    rust_main();
    //printk("module[axruntime]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include "exports_compiler_builtins_generated.h"

int
init_module(void)
{
    //printk("module[compiler_builtins]: init begin ...\n");
    //printk("module[compiler_builtins]: init end!\n");
    return 0;
}

void
rust_begin_unwind(void)
{
}
EXPORT_SYMBOL(rust_begin_unwind);

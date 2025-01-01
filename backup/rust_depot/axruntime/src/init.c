// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <sbi.h>

extern void rust_main();

int
init_module(void)
{
    sbi_puts("module[axruntime]: init begin ...\n");
    rust_main();
    sbi_puts("module[axruntime]: init end!\n");
    return 0;
}

bool axruntime_ready(void) {
    return true;
}
EXPORT_SYMBOL(axruntime_ready);

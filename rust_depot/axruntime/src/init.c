// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>
//#include "exports_axruntime_generated.h"

extern void rust_main();

int
init_module(void)
{
    sbi_puts("module[axruntime]: init begin ...\n");
    rust_main();
    sbi_puts("module[axruntime]: init end!\n");
    return 0;
}

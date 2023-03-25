// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <sbi.h>

extern uintptr_t kernel_size;
extern void rust_main();

int
init_module(void)
{
    sbi_puts("module[axruntime]: init begin ...\n");
    sbi_puts("[");
    sbi_put_u64(kernel_size);
    sbi_puts("]\n");
    rust_main();
    sbi_puts("module[axruntime]: init end!\n");
    return 0;
}

bool axruntime_ready(void) {
    return true;
}
EXPORT_SYMBOL(axruntime_ready);

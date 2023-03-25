// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <sbi.h>

int
init_module(void)
{
    sbi_puts("module[alloc]: init begin ...\n");
    sbi_puts("module[alloc]: init end!\n");
    return 0;
}

char *
__rust_alloc(uintptr_t size, uintptr_t align)
{
    sbi_puts("###### ###### module[alloc] call __rust_alloc!\n");
    sbi_srst_power_off();
    //return __rg_alloc(size, align);
}
EXPORT_SYMBOL(__rust_alloc);

char *
__rust_alloc_zeroed(uintptr_t size, uintptr_t align)
{
    sbi_puts("###### ###### module[alloc] call __rust_alloc_zeroed!\n");
    sbi_srst_power_off();
    //return __rg_alloc_zeroed(size, align);
}
EXPORT_SYMBOL(__rust_alloc_zeroed);

void
__rust_dealloc(char *ptr, uintptr_t size, uintptr_t align)
{
    sbi_puts("###### ###### module[alloc] call __rust_dealloc!\n");
    sbi_srst_power_off();
    //return __rg_dealloc(ptr, size, align);
}
EXPORT_SYMBOL(__rust_dealloc);

char *
__rust_realloc(char *ptr, uintptr_t old_size, uintptr_t _align)
{
    sbi_puts("###### ###### module[alloc] call __rust_realloc!\n");
    sbi_srst_power_off();
    //return __rg_realloc(ptr, size, align);
}
EXPORT_SYMBOL(__rust_realloc);

void
__rust_alloc_error_handler(uintptr_t size, uintptr_t align)
{
    sbi_puts("###### ###### module[alloc] call __rust_alloc_error_handler!\n");
    sbi_srst_power_off();
}

unsigned char __rust_alloc_error_handler_should_panic = 1;

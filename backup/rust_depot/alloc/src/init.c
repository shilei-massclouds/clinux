// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <sbi.h>
#include <rust_alloc.h>

rg_alloc_t rg_alloc;
EXPORT_SYMBOL(rg_alloc);

rg_alloc_t rg_alloc_zeroed;
EXPORT_SYMBOL(rg_alloc_zeroed);

rg_dealloc_t rg_dealloc;
EXPORT_SYMBOL(rg_dealloc);

rg_realloc_t rg_realloc;
EXPORT_SYMBOL(rg_realloc);

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
    SBI_ASSERT_MSG(rg_alloc != NULL, "rg_alloc is NULL!");
    return rg_alloc(size, align);
}
EXPORT_SYMBOL(__rust_alloc);

char *
__rust_alloc_zeroed(uintptr_t size, uintptr_t align)
{
    SBI_ASSERT_MSG(rg_alloc_zeroed != NULL, "rg_alloc_zeroed is NULL!");
    return rg_alloc_zeroed(size, align);
}
EXPORT_SYMBOL(__rust_alloc_zeroed);

void
__rust_dealloc(char *ptr, uintptr_t size, uintptr_t align)
{
    SBI_ASSERT_MSG(rg_dealloc != NULL, "rg_dealloc is NULL!");
    rg_dealloc(ptr, size, align);
}
EXPORT_SYMBOL(__rust_dealloc);

char *
__rust_realloc(char *ptr, uintptr_t old_size,
               uintptr_t align, uintptr_t new_size)
{
    SBI_ASSERT_MSG(rg_realloc != NULL, "rg_realloc is NULL!");
    return rg_realloc(ptr, old_size, align, new_size);
}
EXPORT_SYMBOL(__rust_realloc);

void
__rust_alloc_error_handler(uintptr_t size, uintptr_t align)
{
    sbi_puts("###### ###### module[alloc] call __rust_alloc_error_handler!\n");
    sbi_srst_power_off();
}

unsigned char __rust_alloc_error_handler_should_panic = 1;

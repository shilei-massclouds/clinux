// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_percpu_init(void)
{
    sbi_puts("module[percpu]: init begin ...\n");
    sbi_puts("module[percpu]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_percpu_init);

void *__vmalloc(unsigned long size, gfp_t gfp_mask)
{
    booter_panic("No impl 'percpu'.");
}

void kvfree(const void *addr)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(kvfree);

void* __weak __kmalloc(size_t size, gfp_t flags)
{
    booter_panic("No impl in 'percpu'.");
}
EXPORT_SYMBOL(__kmalloc);

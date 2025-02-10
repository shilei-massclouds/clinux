// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_memblock_init(void)
{
    sbi_puts("module[memblock]: init begin ...\n");
    ENABLE_COMPONENT(early_printk);
    sbi_puts("module[memblock]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_memblock_init);

bool slab_is_available(void)
{
    return false;
}

void *__kmalloc(size_t size, gfp_t flags)
{
    booter_panic("No impl '__kmalloc'.");
}

void kfree(const void *objp)
{
    booter_panic("No impl 'kfree'.");
}

/*
void dump_stack(void)
{
    booter_panic("No impl 'dump_stack'.");
}
*/

void page_init_poison(struct page *page, size_t size)
{
    booter_panic("No impl 'page_init_poison'.");
}

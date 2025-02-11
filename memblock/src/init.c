// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

/*
 * A number of key systems in x86 including ioremap() rely on the assumption
 * that high_memory defines the upper bound on direct map memory, then end
 * of ZONE_NORMAL.  Under CONFIG_DISCONTIG this means that max_low_pfn and
 * highstart_pfn must be the same; there must be no gap between ZONE_NORMAL
 * and ZONE_HIGHMEM.
 */
void *high_memory;
EXPORT_SYMBOL(high_memory);

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

void __weak kfree(const void *objp)
{
    booter_panic("No impl 'kfree'.");
}
EXPORT_SYMBOL(kfree);

void page_init_poison(struct page *page, size_t size)
{
    booter_panic("No impl 'page_init_poison'.");
}

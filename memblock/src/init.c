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

bool __weak slab_is_available(void)
{
    return false;
}
EXPORT_SYMBOL(slab_is_available);

void* __weak __kmalloc(size_t size, gfp_t flags)
{
    booter_panic("No impl '__kmalloc'.");
}
EXPORT_SYMBOL(__kmalloc);

void __weak kfree(const void *objp)
{
    booter_panic("No impl 'kfree'.");
}
EXPORT_SYMBOL(kfree);

__weak void page_init_poison(struct page *page, size_t size)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL_GPL(page_init_poison);

__weak struct kmem_cache *
kmem_cache_create(const char *name, unsigned int size, unsigned int align,
        slab_flags_t flags, void (*ctor)(void *))
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(kmem_cache_create);

__weak void kmem_cache_free(struct kmem_cache *s, void *x)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(kmem_cache_free);

__weak void __init memblock_free_pages(struct page *page, unsigned long pfn,
							unsigned int order)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(memblock_free_pages);

atomic_long_t _totalram_pages __read_mostly;
EXPORT_SYMBOL(_totalram_pages);

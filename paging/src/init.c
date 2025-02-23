// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "internal.h"
#include "../../booter/src/booter.h"

unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)]
                            __page_aligned_bss;
EXPORT_SYMBOL(empty_zero_page);

int
cl_paging_init(void)
{
    sbi_puts("module[paging]: init begin ...\n");
    sbi_puts("module[paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_paging_init);

void dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'dump_page'.");
}
EXPORT_SYMBOL(dump_page);

void __weak __free_pages(struct page *page, unsigned int order)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__free_pages);

struct page * __weak
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order, int preferred_nid,
							nodemask_t *nodemask)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__alloc_pages_nodemask);

#ifdef CONFIG_VM_EVENT_COUNTERS
DEFINE_PER_CPU(struct vm_event_state, vm_event_states) = {{0}};
EXPORT_PER_CPU_SYMBOL(vm_event_states);
#endif /* CONFIG_VM_EVENT_COUNTERS */

__weak void *__init alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long low_limit,
				     unsigned long high_limit)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(alloc_large_system_hash);

atomic_long_t vm_zone_stat[NR_VM_ZONE_STAT_ITEMS] __cacheline_aligned_in_smp;
EXPORT_SYMBOL(vm_zone_stat);
atomic_long_t vm_numa_stat[NR_VM_NUMA_STAT_ITEMS] __cacheline_aligned_in_smp;
EXPORT_SYMBOL(vm_numa_stat);
atomic_long_t vm_node_stat[NR_VM_NODE_STAT_ITEMS] __cacheline_aligned_in_smp;
EXPORT_SYMBOL(vm_node_stat);

__weak bool gfp_pfmemalloc_allowed(gfp_t gfp_mask)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(gfp_pfmemalloc_allowed);

__weak void free_pages(unsigned long addr, unsigned int order)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(free_pages);

__weak unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__get_free_pages);

gfp_t gfp_allowed_mask __read_mostly = GFP_BOOT_MASK;
EXPORT_SYMBOL(gfp_allowed_mask);

__weak struct kmem_cache *
kmem_cache_create_usercopy(const char *name,
		  unsigned int size, unsigned int align,
		  slab_flags_t flags,
		  unsigned int useroffset, unsigned int usersize,
		  void (*ctor)(void *))
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kmem_cache_create_usercopy);

__weak unsigned long nr_free_buffer_pages(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(nr_free_buffer_pages);

long congestion_wait(int sync, long timeout)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(congestion_wait);

__weak void warn_alloc(gfp_t gfp_mask, nodemask_t *nodemask, const char *fmt, ...)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(warn_alloc);

__weak void free_compound_page(struct page *page)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(free_compound_page);

__weak unsigned long get_zeroed_page(gfp_t gfp_mask)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(get_zeroed_page);

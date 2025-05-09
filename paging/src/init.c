// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/compaction.h>
#include <cl_hook.h>
#include "internal.h"
#include "../../booter/src/booter.h"

unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)]
                            __page_aligned_bss;
EXPORT_SYMBOL(empty_zero_page);

extern void __init setup_vm_final(void);
extern void setup_zero_page(void);
extern void __init zone_sizes_init(void);

int
cl_paging_init(void)
{
    sbi_puts("module[paging]: init begin ...\n");
    REQUIRE_COMPONENT(bootmem);

    setup_vm_final();
    sparse_init();
    setup_zero_page();
    zone_sizes_init();

    sbi_puts("module[paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_paging_init);

DEFINE_ENABLE_FUNC(paging);

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

__weak void free_unref_page_list(struct list_head *list)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(free_unref_page_list);

__weak void drain_local_pages(struct zone *zone)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(drain_local_pages);

__weak void free_unref_page(struct page *page)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(free_unref_page);

__weak bool zone_watermark_ok_safe(struct zone *z, unsigned int order,
			unsigned long mark, int highest_zoneidx)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(zone_watermark_ok_safe);

__weak void split_page(struct page *page, unsigned int order)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(split_page);

__weak void drain_all_pages(struct zone *zone)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(drain_all_pages);

__weak void reset_isolation_suitable(pg_data_t *pgdat)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(reset_isolation_suitable);

__weak void wakeup_kcompactd(pg_data_t *pgdat, int order, int highest_zoneidx)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(wakeup_kcompactd);

__weak
enum compact_result compaction_suitable(struct zone *zone, int order,
					unsigned int alloc_flags,
					int highest_zoneidx)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(compaction_suitable);

__weak int access_remote_vm(struct mm_struct *mm, unsigned long addr,
		void *buf, int len, unsigned int gup_flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(access_remote_vm);

__weak int copy_page_range(struct mm_struct *dst_mm, struct mm_struct *src_mm,
		    struct vm_area_struct *vma, struct vm_area_struct *new)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(copy_page_range);

__weak vm_fault_t alloc_set_pte(struct vm_fault *vmf, struct page *page)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(alloc_set_pte);

__weak int __pmd_alloc(struct mm_struct *mm, pud_t *pud,
                        unsigned long address)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__pmd_alloc);

__weak void unmap_mapping_range(struct address_space *mapping,
		loff_t const holebegin, loff_t const holelen, int even_cows)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unmap_mapping_range);

__weak int __pte_alloc_kernel(pmd_t *pmd)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__pte_alloc_kernel);

__weak int remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
		    unsigned long pfn, unsigned long size, pgprot_t prot)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(remap_pfn_range);

__weak void unmap_mapping_pages(struct address_space *mapping, pgoff_t start,
		pgoff_t nr, bool even_cows)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unmap_mapping_pages);

__weak vm_fault_t handle_mm_fault(struct vm_area_struct *vma, unsigned long address,
			   unsigned int flags, struct pt_regs *regs)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL_GPL(handle_mm_fault);

unsigned long zero_pfn __read_mostly;
EXPORT_SYMBOL(zero_pfn);

__weak struct page *vm_normal_page(struct vm_area_struct *vma, unsigned long addr,
			    pte_t pte)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(vm_normal_page);

__weak int __pte_alloc(struct mm_struct *mm, pmd_t *pmd)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__pte_alloc);

__weak void unmap_vmas(struct mmu_gather *tlb,
		struct vm_area_struct *vma, unsigned long start_addr,
		unsigned long end_addr)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unmap_vmas);

__weak void free_pgtables(struct mmu_gather *tlb, struct vm_area_struct *vma,
		unsigned long floor, unsigned long ceiling)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(free_pgtables);

__weak void unmap_page_range(struct mmu_gather *tlb,
			     struct vm_area_struct *vma,
			     unsigned long addr, unsigned long end,
			     struct zap_details *details)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unmap_page_range);

unsigned int _debug_guardpage_minorder;
EXPORT_SYMBOL(_debug_guardpage_minorder);

#ifdef CONFIG_INIT_ON_ALLOC_DEFAULT_ON
DEFINE_STATIC_KEY_TRUE(init_on_alloc);
#else
DEFINE_STATIC_KEY_FALSE(init_on_alloc);
#endif
EXPORT_SYMBOL(init_on_alloc);

#ifdef CONFIG_INIT_ON_FREE_DEFAULT_ON
DEFINE_STATIC_KEY_TRUE(init_on_free);
#else
DEFINE_STATIC_KEY_FALSE(init_on_free);
#endif
EXPORT_SYMBOL(init_on_free);

void setup_zero_page(void)
{
    memset((void *)empty_zero_page, 0, PAGE_SIZE);
}
EXPORT_SYMBOL(setup_zero_page);

void __init zone_sizes_init(void)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES] = { 0, };

#ifdef CONFIG_ZONE_DMA32
    max_zone_pfns[ZONE_DMA32] = PFN_DOWN(min(4UL * SZ_1G,
            (unsigned long) PFN_PHYS(max_low_pfn)));
#endif
    max_zone_pfns[ZONE_NORMAL] = max_low_pfn;

    free_area_init(max_zone_pfns);
}
EXPORT_SYMBOL(zone_sizes_init);

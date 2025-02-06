// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/mmzone.h>
#include <linux/sizes.h>
#include <linux/memblock.h>
#include "internal.h"
#include "../../booter/src/booter.h"

extern void setup_vm_final(void);
extern void free_area_init(unsigned long *max_zone_pfn);

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

static void setup_zero_page(void)
{
    memset((void *)empty_zero_page, 0, PAGE_SIZE);
}

static void __init zone_sizes_init(void)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES] = { 0, };

#ifdef CONFIG_ZONE_DMA32
    max_zone_pfns[ZONE_DMA32] = PFN_DOWN(min(4UL * SZ_1G,
            (unsigned long) PFN_PHYS(max_low_pfn)));
#endif
    max_zone_pfns[ZONE_NORMAL] = max_low_pfn;

    free_area_init(max_zone_pfns);
}

static void __init resource_init(void)
{
    struct memblock_region *region;

    for_each_memblock(memory, region) {
        struct resource *res;

        res = memblock_alloc(sizeof(struct resource), SMP_CACHE_BYTES);
        if (!res)
            panic("%s: Failed to allocate %zu bytes\n", __func__,
                  sizeof(struct resource));

        if (memblock_is_nomap(region)) {
            res->name = "reserved";
            res->flags = IORESOURCE_MEM;
        } else {
            res->name = "System RAM";
            res->flags = IORESOURCE_SYSTEM_RAM | IORESOURCE_BUSY;
        }
        res->start = __pfn_to_phys(memblock_region_memory_base_pfn(region));
        res->end = __pfn_to_phys(memblock_region_memory_end_pfn(region)) - 1;

        request_resource(&iomem_resource, res);
    }
}

void __init paging_init(void)
{
    setup_vm_final();
    sparse_init();
    setup_zero_page();
    zone_sizes_init();
    resource_init();
}
EXPORT_SYMBOL(paging_init);

#ifdef CONFIG_DEBUG_MEMORY_INIT

int __meminitdata mminit_loglevel;

#ifndef SECTIONS_SHIFT
#define SECTIONS_SHIFT  0
#endif

void __init mminit_verify_pageflags_layout(void)
{
	int shift, width;
	unsigned long or_mask, add_mask;

	shift = 8 * sizeof(unsigned long);
	width = shift - SECTIONS_WIDTH - NODES_WIDTH - ZONES_WIDTH
		- LAST_CPUPID_SHIFT - KASAN_TAG_WIDTH;
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_widths",
		"Section %d Node %d Zone %d Lastcpupid %d Kasantag %d Flags %d\n",
		SECTIONS_WIDTH,
		NODES_WIDTH,
		ZONES_WIDTH,
		LAST_CPUPID_WIDTH,
		KASAN_TAG_WIDTH,
		NR_PAGEFLAGS);
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_shifts",
		"Section %d Node %d Zone %d Lastcpupid %d Kasantag %d\n",
		SECTIONS_SHIFT,
		NODES_SHIFT,
		ZONES_SHIFT,
		LAST_CPUPID_SHIFT,
		KASAN_TAG_WIDTH);
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_pgshifts",
		"Section %lu Node %lu Zone %lu Lastcpupid %lu Kasantag %lu\n",
		(unsigned long)SECTIONS_PGSHIFT,
		(unsigned long)NODES_PGSHIFT,
		(unsigned long)ZONES_PGSHIFT,
		(unsigned long)LAST_CPUPID_PGSHIFT,
		(unsigned long)KASAN_TAG_PGSHIFT);
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_nodezoneid",
		"Node/Zone ID: %lu -> %lu\n",
		(unsigned long)(ZONEID_PGOFF + ZONEID_SHIFT),
		(unsigned long)ZONEID_PGOFF);
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_usage",
		"location: %d -> %d layout %d -> %d unused %d -> %d page-flags\n",
		shift, width, width, NR_PAGEFLAGS, NR_PAGEFLAGS, 0);
#ifdef NODE_NOT_IN_PAGE_FLAGS
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_nodeflags",
		"Node not in page flags");
#endif
#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
	mminit_dprintk(MMINIT_TRACE, "pageflags_layout_nodeflags",
		"Last cpupid not in page flags");
#endif

	if (SECTIONS_WIDTH) {
		shift -= SECTIONS_WIDTH;
		BUG_ON(shift != SECTIONS_PGSHIFT);
	}
	if (NODES_WIDTH) {
		shift -= NODES_WIDTH;
		BUG_ON(shift != NODES_PGSHIFT);
	}
	if (ZONES_WIDTH) {
		shift -= ZONES_WIDTH;
		BUG_ON(shift != ZONES_PGSHIFT);
	}

	/* Check for bitmask overlaps */
	or_mask = (ZONES_MASK << ZONES_PGSHIFT) |
			(NODES_MASK << NODES_PGSHIFT) |
			(SECTIONS_MASK << SECTIONS_PGSHIFT);
	add_mask = (ZONES_MASK << ZONES_PGSHIFT) +
			(NODES_MASK << NODES_PGSHIFT) +
			(SECTIONS_MASK << SECTIONS_PGSHIFT);
	BUG_ON(or_mask != add_mask);
}

#endif /* CONFIG_DEBUG_MEMORY_INIT */

void dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'dump_page'.");
}

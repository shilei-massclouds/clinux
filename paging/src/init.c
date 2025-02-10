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

void dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'dump_page'.");
}
EXPORT_SYMBOL(dump_page);

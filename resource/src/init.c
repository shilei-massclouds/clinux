// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include "../../booter/src/booter.h"

int
cl_resource_init(void)
{
    sbi_puts("module[resource]: init begin ...\n");
    sbi_puts("module[resource]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_resource_init);

void __init resource_init(void)
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
EXPORT_SYMBOL(resource_init);

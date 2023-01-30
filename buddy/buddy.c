// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <bug.h>
#include <gfp.h>
#include <log2.h>
#include <page.h>
#include <sizes.h>
#include <export.h>
#include <kernel.h>
#include <string.h>
#include <mmzone.h>
#include <printk.h>
#include <highmem.h>
#include <memblock.h>
#include <mm_types.h>
#include <page_ref.h>
#include <page-flags.h>

#define RESERVED_CHUNK_SIZE (PAGE_SIZE << 3)

extern void (*reserve_bootmem_region_fn)(phys_addr_t, phys_addr_t);
extern void (*free_pages_core_fn)(struct page *, unsigned int);
extern struct pglist_data contig_page_data;
extern struct page *mem_map;

extern unsigned long max_low_pfn;

size_t reserved_chunk_size;
EXPORT_SYMBOL(reserved_chunk_size);
void *reserved_chunk_ptr;
EXPORT_SYMBOL(reserved_chunk_ptr);

unsigned long nr_kernel_pages;
EXPORT_SYMBOL(nr_kernel_pages);

static struct per_cpu_pageset boot_pageset;

static unsigned long
arch_zone_lowest_possible_pfn[MAX_NR_ZONES];

static unsigned long
arch_zone_highest_possible_pfn[MAX_NR_ZONES];

static char * const zone_names[MAX_NR_ZONES] = {
     "DMA32",
     "Normal",
     "Movable",
};

/**
 * find_min_pfn_with_active_regions - Find the minimum PFN registered
 *
 * Return: the minimum PFN based on information provided via
 * memblock_set_node().
 */
unsigned long
find_min_pfn_with_active_regions(void)
{
    return PHYS_PFN(memblock_start_of_DRAM());
}

void
get_pfn_range_for_nid(unsigned long *start_pfn, unsigned long *end_pfn)
{
    int i;
    unsigned long this_start_pfn;
    unsigned long this_end_pfn;

    *start_pfn = -1UL;
    *end_pfn = 0;

    for_each_mem_pfn_range(i, &this_start_pfn, &this_end_pfn) {
        *start_pfn = min(*start_pfn, this_start_pfn);
        *end_pfn = max(*end_pfn, this_end_pfn);
    }

    if (*start_pfn == -1UL)
        *start_pfn = 0;
}

static unsigned long
zone_spanned_pages_in_node(unsigned long zone_type,
                           unsigned long node_start_pfn,
                           unsigned long node_end_pfn,
                           unsigned long *zone_start_pfn,
                           unsigned long *zone_end_pfn)
{
    unsigned long zone_low = arch_zone_lowest_possible_pfn[zone_type];
    unsigned long zone_high = arch_zone_highest_possible_pfn[zone_type];
    /* When hotadd a new node from cpu_up(), the node should be empty */
    if (!node_start_pfn && !node_end_pfn)
        return 0;

    /* Get the start and end of the zone */
    *zone_start_pfn = clamp(node_start_pfn, zone_low, zone_high);
    *zone_end_pfn = clamp(node_end_pfn, zone_low, zone_high);

    /* Check that this node has pages within the zone's required range */
    if (*zone_end_pfn < node_start_pfn || *zone_start_pfn > node_end_pfn)
        return 0;

    /* Move the zone boundaries inside the node if necessary */
    *zone_end_pfn = min(*zone_end_pfn, node_end_pfn);
    *zone_start_pfn = max(*zone_start_pfn, node_start_pfn);

    /* Return the spanned pages */
    return *zone_end_pfn - *zone_start_pfn;
}

/*
 * Return the number of holes in a range on a node. If nid is MAX_NUMNODES,
 * then all holes in the requested range will be accounted for.
 */
unsigned long
__absent_pages_in_range(unsigned long range_start_pfn,
                        unsigned long range_end_pfn)
{
    unsigned long nr_absent = range_end_pfn - range_start_pfn;
    unsigned long start_pfn, end_pfn;
    int i;

    for_each_mem_pfn_range(i, &start_pfn, &end_pfn) {
        start_pfn = clamp(start_pfn, range_start_pfn, range_end_pfn);
        end_pfn = clamp(end_pfn, range_start_pfn, range_end_pfn);
        nr_absent -= end_pfn - start_pfn;
    }

    return nr_absent;
}

/* Return the number of page frames in holes in a zone on a node */
static unsigned long
zone_absent_pages_in_node(unsigned long zone_type,
                          unsigned long node_start_pfn,
                          unsigned long node_end_pfn)
{
    unsigned long zone_low = arch_zone_lowest_possible_pfn[zone_type];
    unsigned long zone_high = arch_zone_highest_possible_pfn[zone_type];
    unsigned long zone_start_pfn, zone_end_pfn;

    /* When hotadd a new node from cpu_up(), the node should be empty */
    if (!node_start_pfn && !node_end_pfn)
        return 0;

    zone_start_pfn = clamp(node_start_pfn, zone_low, zone_high);
    zone_end_pfn = clamp(node_end_pfn, zone_low, zone_high);

    return __absent_pages_in_range(zone_start_pfn, zone_end_pfn);
}

static void
calculate_node_totalpages(struct pglist_data *pgdat,
                          unsigned long node_start_pfn,
                          unsigned long node_end_pfn)
{
    enum zone_type i;
    unsigned long totalpages = 0;
    unsigned long realtotalpages = 0;

    for (i = 0; i < MAX_NR_ZONES; i++) {
        struct zone *zone = pgdat->node_zones + i;
        unsigned long zone_start_pfn, zone_end_pfn;
        unsigned long spanned, absent;
        unsigned long size, real_size;

        spanned = zone_spanned_pages_in_node(i,
                                             node_start_pfn, node_end_pfn,
                                             &zone_start_pfn, &zone_end_pfn);

        absent = zone_absent_pages_in_node(i, node_start_pfn, node_end_pfn);

        size = spanned;
        real_size = size - absent;

        if (size)
            zone->zone_start_pfn = zone_start_pfn;
        else
            zone->zone_start_pfn = 0;

        zone->spanned_pages = size;
        zone->present_pages = real_size;

        totalpages += size;
        realtotalpages += real_size;
    }

    pgdat->node_spanned_pages = totalpages;
    pgdat->node_present_pages = realtotalpages;
    printk("On node 0 totalpages: %lu\n", realtotalpages);
}

static void
alloc_node_mem_map(struct pglist_data *pgdat)
{
    unsigned long start = 0;
    unsigned long offset = 0;

    /* Skip empty nodes */
    if (!pgdat->node_spanned_pages)
        return;

    start = pgdat->node_start_pfn & ~(MAX_ORDER_NR_PAGES - 1);
    offset = pgdat->node_start_pfn - start;

    if (!pgdat->node_mem_map) {
        unsigned long size, end;
        struct page *map;

        /*
         * The zone's endpoints aren't required to be MAX_ORDER
         * aligned but the node_mem_map endpoints must be in order
         * for the buddy allocator to function correctly.
         */
        end = pgdat_end_pfn(pgdat);
        end = ALIGN(end, MAX_ORDER_NR_PAGES);
        size = (end - start) * sizeof(struct page);
        map = memblock_alloc_node(size, SMP_CACHE_BYTES);
        if (!map)
            panic("Failed to allocate %ld bytes for node 0 memory map\n",
                  size);
        pgdat->node_mem_map = map + offset;
    }

    /*
     * With no DISCONTIG, the global mem_map is just set as node 0's
     */
    if (pgdat == NODE_DATA(0)) {
        mem_map = NODE_DATA(0)->node_mem_map;
        if (page_to_pfn(mem_map) != pgdat->node_start_pfn)
            mem_map -= offset;
    }
}

static void
zone_init_free_lists(struct zone *zone)
{
    unsigned int order;

    for_each_order(order) {
        INIT_LIST_HEAD(&zone->free_area[order].free_list);
        zone->free_area[order].nr_free = 0;
    }
}

void
init_currently_empty_zone(struct zone *zone,
                          unsigned long zone_start_pfn,
                          unsigned long size)
{
    struct pglist_data *pgdat = zone->zone_pgdat;
    int zone_idx = zone_idx(zone) + 1;

    if (zone_idx > pgdat->nr_zones)
        pgdat->nr_zones = zone_idx;

    zone->zone_start_pfn = zone_start_pfn;

    printk("Initialising map zone %lu pfns %lu -> %lu\n",
           (unsigned long)zone_idx(zone),
           zone_start_pfn, (zone_start_pfn + size));

    zone_init_free_lists(zone);
    zone->initialized = 1;
}

static void
zone_pcp_init(struct zone *zone)
{
    /*
     * per cpu subsystem is not up at this point. The following code
     * relies on the ability of the linker to provide the
     * offset of a (static) per cpu variable into the per cpu area.
     */
    zone->pageset = &boot_pageset;
}

static void
zone_init_internals(struct zone *zone,
                    enum zone_type idx,
                    unsigned long remaining_pages)
{
    atomic_long_set(&zone->managed_pages, remaining_pages);
    zone->name = zone_names[idx];
    zone->zone_pgdat = NODE_DATA(0);
    zone_pcp_init(zone);
}

static unsigned long
calc_memmap_size(unsigned long spanned_pages)
{
    return PAGE_ALIGN(spanned_pages * sizeof(struct page)) >> PAGE_SHIFT;
}

static void
free_area_init_core(struct pglist_data *pgdat)
{
    enum zone_type j;

    for (j = 0; j < MAX_NR_ZONES; j++) {
        unsigned long size;
        unsigned long freesize;
        unsigned long memmap_pages;
        struct zone *zone = pgdat->node_zones + j;
        unsigned long zone_start_pfn = zone->zone_start_pfn;

        size = zone->spanned_pages;
        freesize = zone->present_pages;
        nr_kernel_pages += freesize;

        memmap_pages = calc_memmap_size(size);
        if (freesize >= memmap_pages) {
            freesize -= memmap_pages;
            if (memmap_pages)
                printk("  %s zone: %lu pages used for memmap\n",
                       zone_names[j], memmap_pages);
        } else {
            printk("  %s zone: %lu pages exceeds freesize %lu\n",
                   zone_names[j], memmap_pages, freesize);
        }

        /*
         * Set an approximate value for lowmem here, it will be adjusted
         * when the bootmem allocator frees pages into the buddy system.
         * And all highmem pages will be managed by the buddy system.
         */
        zone_init_internals(zone, j, freesize);

        if (!size)
            continue;

        init_currently_empty_zone(zone, zone_start_pfn, size);
    }
}

static void
free_area_init_node(void)
{
    unsigned long start_pfn = 0;
    unsigned long end_pfn = 0;
    pg_data_t *pgdat = NODE_DATA(0);

    /* pg_data_t should be reset to zero when it's allocated */
    BUG_ON(pgdat->nr_zones || pgdat->kswapd_highest_zoneidx);

    get_pfn_range_for_nid(&start_pfn, &end_pfn);

    pgdat->node_start_pfn = start_pfn;

    printk("Initmem setup [mem %lx-%lx]\n",
           (u64)start_pfn << PAGE_SHIFT,
           end_pfn ? ((u64)end_pfn << PAGE_SHIFT) - 1 : 0);

    calculate_node_totalpages(pgdat, start_pfn, end_pfn);

    alloc_node_mem_map(pgdat);
    free_area_init_core(pgdat);
}

void
free_area_init(unsigned long *max_zone_pfn)
{
    int i;
    unsigned long start_pfn;
    unsigned long end_pfn;
    pg_data_t *pgdat = NODE_DATA(0);

    memset(arch_zone_lowest_possible_pfn, 0,
           sizeof(arch_zone_lowest_possible_pfn));
    memset(arch_zone_highest_possible_pfn, 0,
           sizeof(arch_zone_highest_possible_pfn));

    start_pfn = find_min_pfn_with_active_regions();

    for (i = 0; i < MAX_NR_ZONES; i++) {
        if (i == ZONE_MOVABLE)
            continue;

        end_pfn = max(max_zone_pfn[i], start_pfn);
        arch_zone_lowest_possible_pfn[i] = start_pfn;
        arch_zone_highest_possible_pfn[i] = end_pfn;

        start_pfn = end_pfn;
    }

    /* Print out the zone ranges */
    printk("Zone ranges:\n");
    for (i = 0; i < MAX_NR_ZONES; i++) {
        if (i == ZONE_MOVABLE)
            continue;

        if (arch_zone_lowest_possible_pfn[i] ==
            arch_zone_highest_possible_pfn[i])
            printk("%s empty\n", zone_names[i]);
        else
            printk("%s [mem %lx-%lx)\n",
                   zone_names[i],
                   (u64)arch_zone_lowest_possible_pfn[i] << PAGE_SHIFT,
                   ((u64)arch_zone_highest_possible_pfn[i] << PAGE_SHIFT) - 1);
    }

    free_area_init_node();
}

static void
zone_sizes_init(void)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES] = {0, };

    max_zone_pfns[ZONE_DMA32] = min(PFN_DOWN(4UL * SZ_1G), max_low_pfn);
    max_zone_pfns[ZONE_NORMAL] = max_low_pfn;

    free_area_init(max_zone_pfns);
}

void
reserve_bootmem_region(phys_addr_t start, phys_addr_t end)
{
    unsigned long start_pfn = PFN_DOWN(start);
    unsigned long end_pfn = PFN_UP(end);

    for (; start_pfn < end_pfn; start_pfn++) {
        if (pfn_valid(start_pfn)) {
            struct page *page = pfn_to_page(start_pfn);

            /* Avoid false-positive PageTail() */
            INIT_LIST_HEAD(&page->lru);

            /*
             * no need for atomic set_bit because the struct
             * page is not visible yet so nobody should
             * access it yet.
             */
            __SetPageReserved(page);
        }
    }
}

static inline bool
page_is_buddy(struct page *page, struct page *buddy, unsigned int order)
{
    if (!PageBuddy(buddy))
        return false;

    if (page_order(buddy) != order)
        return false;

    /*
     * zone check is done late to avoid uselessly calculating
     * zone/node ids for pages that could never merge.
     */
    if (page_zone_id(page) != page_zone_id(buddy))
        return false;

    return true;
}

static inline void set_page_order(struct page *page, unsigned int order)
{
    set_page_private(page, order);
    __SetPageBuddy(page);
}

static inline bool
buddy_merge_likely(unsigned long pfn,
                   unsigned long buddy_pfn,
                   struct page *page,
                   unsigned int order)
{
    struct page *higher_page, *higher_buddy;
    unsigned long combined_pfn;

    if (order >= MAX_ORDER - 2)
        return false;

    combined_pfn = buddy_pfn & pfn;
    higher_page = page + (combined_pfn - pfn);
    buddy_pfn = __find_buddy_pfn(combined_pfn, order + 1);
    higher_buddy = higher_page + (buddy_pfn - combined_pfn);

    return page_is_buddy(higher_page, higher_buddy, order + 1);
}

/* Used for pages not on another list */
static inline void
add_to_free_list(struct page *page,
                 struct zone *zone,
                 unsigned int order)
{
    struct free_area *area = &zone->free_area[order];

    list_add(&page->lru, &area->free_list);
    area->nr_free++;
}

/* Used for pages not on another list */
static inline void
add_to_free_list_tail(struct page *page,
                      struct zone *zone,
                      unsigned int order)
{
    struct free_area *area = &zone->free_area[order];

    list_add_tail(&page->lru, &area->free_list);
    area->nr_free++;
}

static inline void
del_page_from_free_list(struct page *page,
                        struct zone *zone,
                        unsigned int order)
{
    list_del(&page->lru);
    __ClearPageBuddy(page);
    set_page_private(page, 0);
    zone->free_area[order].nr_free--;
}

/*
 * Freeing function for a buddy system allocator.
 *
 * The concept of a buddy system is to maintain direct-mapped table
 * (containing bit values) for memory blocks of various "orders".
 * The bottom level table contains the map for the smallest allocatable
 * units of memory (here, pages), and each level above it describes
 * pairs of units from the levels below, hence, "buddies".
 * At a high level, all that happens here is marking the table entry
 * at the bottom level available, and propagating the changes upward
 * as necessary, plus some accounting needed to play nicely with other
 * parts of the VM system.
 * At each level, we keep a list of pages, which are heads of continuous
 * free pages of length of (1 << order) and marked with PageBuddy.
 * Page's order is recorded in page_private(page) field.
 * So when we are allocating or freeing one, we can derive the state of the
 * other.  That is, if we allocate a small block, and both were
 * free, the remainder of the region must be split into blocks.
 * If a block is freed, and its buddy is also free, then this
 * triggers coalescing into a block of larger size.
 *
 * -- nyc
 */

static inline void
__free_one_page(struct page *page,
                unsigned long pfn,
                struct zone *zone,
                unsigned int order,
                bool report)
{
    unsigned long buddy_pfn;
    unsigned long combined_pfn;
    struct page *buddy;
    bool to_tail;

    while (order < MAX_ORDER - 1) {
        buddy_pfn = __find_buddy_pfn(pfn, order);
        buddy = page + (buddy_pfn - pfn);

        if (!page_is_buddy(page, buddy, order))
            goto done_merging;

        del_page_from_free_list(buddy, zone, order);

        combined_pfn = buddy_pfn & pfn;
        page = page + (combined_pfn - pfn);
        pfn = combined_pfn;
        order++;
    }

 done_merging:
    set_page_order(page, order);

    to_tail = buddy_merge_likely(pfn, buddy_pfn, page, order);
    if (to_tail)
        add_to_free_list_tail(page, zone, order);
    else
        add_to_free_list(page, zone, order);
}

static void
free_one_page(struct zone *zone,
              struct page *page,
              unsigned long pfn,
              unsigned int order)
{
    __free_one_page(page, pfn, zone, order, true);
}


static void
__free_pages_ok(struct page *page, unsigned int order)
{
    unsigned long flags;
    unsigned long pfn = page_to_pfn(page);

    free_one_page(page_zone(page), page, pfn, order);
}

static inline void
free_the_page(struct page *page, unsigned int order)
{
    if (order == 0)     /* Via pcp? */
        __free_pages_ok(page, order);
    else
        __free_pages_ok(page, order);
}

void
__free_pages(struct page *page, unsigned int order)
{
    if (put_page_testzero(page))
        free_the_page(page, order);
}
EXPORT_SYMBOL(__free_pages);

void
__free_pages_core(struct page *page, unsigned int order)
{
    unsigned int loop;
    unsigned int nr_pages = 1 << order;
    struct page *p = page;

    for (loop = 0; loop < nr_pages; loop++, p++) {
        __ClearPageReserved(p);
        set_page_count(p, 0);
    }

    atomic_long_add(nr_pages, &page_zone(page)->managed_pages);
    set_page_refcounted(page);
    __free_pages(page, order);
}

static inline bool
prepare_alloc_pages(gfp_t gfp_mask, unsigned int order,
                    struct alloc_context *ac)
{
    ac->highest_zoneidx = gfp_zone(gfp_mask);
    ac->zonelist = node_zonelist(gfp_mask);
    return true;
}

/* Determine whether to spread dirty pages and what the first usable zone */
static inline void
finalise_ac(gfp_t gfp_mask, struct alloc_context *ac)
{
    /*
     * The preferred zone is used for statistics but crucially it is
     * also used as the starting point for the zonelist iterator. It
     * may get reset for allocations that ignore memory policies.
     */
    ac->preferred_zoneref = first_zones_zonelist(ac->zonelist,
                                                 ac->highest_zoneidx);
}

static inline void
expand(struct zone *zone, struct page *page, int low, int high)
{
    unsigned long size = 1 << high;

    while (high > low) {
        high--;
        size >>= 1;

        add_to_free_list(&page[size], zone, high);
        set_page_order(&page[size], high);
    }
}

/*
 * Go through the free lists and remove
 * the smallest available page from the freelists
 */
static __always_inline struct page *
__rmqueue_smallest(struct zone *zone, unsigned int order)
{
    unsigned int current_order;
    struct free_area *area;
    struct page *page;

    /* Find a page of the appropriate size in the preferred list */
    for (current_order = order; current_order < MAX_ORDER; ++current_order) {
        area = &(zone->free_area[current_order]);
        page = get_page_from_free_area(area);
        if (!page)
            continue;

        del_page_from_free_list(page, zone, current_order);
        expand(zone, page, order, current_order);
        return page;
    }

    return NULL;
}

/*
 * Do the hard work of removing an element from the buddy allocator.
 * Call me with the zone->lock already held.
 */
static __always_inline struct page *
__rmqueue(struct zone *zone, unsigned int order, unsigned int alloc_flags)
{
    struct page *page;

    page = __rmqueue_smallest(zone, order);
    if (unlikely(!page)) {
        panic("bad __rmqueue_smallest! order(%u)", order);
    }

    return page;
}

/*
 * Obtain a specified number of elements from the buddy allocator,
 * all under a single hold of the lock, for efficiency.
 * Add them to the supplied list.
 * Returns the number of new pages which were placed at *list.
 */
static int
rmqueue_bulk(struct zone *zone,
             unsigned int order,
             unsigned long count,
             struct list_head *list,
             unsigned int alloc_flags)
{
    int i;
    int alloced = 0;

    for (i = 0; i < count; ++i) {
        struct page *page = __rmqueue(zone, order, alloc_flags);
        if (unlikely(page == NULL))
            break;

        /*
         * Split buddy pages returned by expand() are received here in
         * physical page order. The page is added to the tail of
         * caller's list. From the callers perspective, the linked list
         * is ordered by page number under some conditions. This is
         * useful for IO devices that can forward direction from the
         * head, thus also in the physical page order. This is useful
         * for IO devices that can merge IO requests if the physical
         * pages are ordered properly.
         */
        list_add_tail(&page->lru, list);
        alloced++;
    }

    return alloced;
}

/* Remove page from the per-cpu list, caller must protect the list */
static struct page *
__rmqueue_pcplist(struct zone *zone,
                  unsigned int alloc_flags,
                  struct per_cpu_pages *pcp,
                  struct list_head *list)
{
    struct page *page;

    if (list_empty(list)) {
        pcp->count += rmqueue_bulk(zone, 0, pcp->batch, list, alloc_flags);
        if (unlikely(list_empty(list)))
            return NULL;
    }

    page = list_first_entry(list, struct page, lru);
    list_del(&page->lru);
    pcp->count--;
    return page;
}

static struct page *
rmqueue_pcplist(struct zone *preferred_zone,
                struct zone *zone,
                gfp_t gfp_flags,
                unsigned int alloc_flags)
{
    struct per_cpu_pages *pcp;
    struct list_head *list;

    pcp = &zone->pageset->pcp;
    list = &pcp->lists;
    return __rmqueue_pcplist(zone, alloc_flags, pcp, list);
}

static inline struct page *
rmqueue(struct zone *preferred_zone,
        struct zone *zone,
        unsigned int order,
        gfp_t gfp_flags,
        unsigned int alloc_flags)
{
    struct page *page = NULL;

    if (likely(order == 0)) {
        page = rmqueue_pcplist(preferred_zone, zone, gfp_flags, alloc_flags);
        goto out;
    }

    page = __rmqueue(zone, order, alloc_flags);

 out:
    if (!page)
        panic("bad order(%u)\n", order);

    return page;
}

static void kernel_init_free_pages(struct page *page, int numpages)
{
    int i;

    for (i = 0; i < numpages; i++)
        clear_highpage(page + i);
}

static void
prep_new_page(struct page *page, unsigned int order,
              gfp_t gfp_flags, unsigned int alloc_flags)
{
    if (want_init_on_alloc(gfp_flags))
        kernel_init_free_pages(page, 1 << order);
}

/*
 * get_page_from_freelist goes through the zonelist trying to allocate
 * a page.
 */
static struct page *
get_page_from_freelist(gfp_t gfp_mask,
                       unsigned int order,
                       int alloc_flags,
                       const struct alloc_context *ac)
{
    struct zoneref *z;
    struct zone *zone;

    z = ac->preferred_zoneref;

    for_next_zone_zonelist_nodemask(zone, z, ac->zonelist,
                                    ac->highest_zoneidx) {
        struct page *page;

        page = rmqueue(ac->preferred_zoneref->zone, zone, order,
                       gfp_mask, alloc_flags);
        if (page) {
            prep_new_page(page, order, gfp_mask, alloc_flags);

            return page;
        }

        panic("bad rmqueue!\n");
    }

    return NULL;
}

struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;
    unsigned int alloc_flags = ALLOC_WMARK_LOW;
    struct alloc_context ac = {};

    if (unlikely(order >= MAX_ORDER)) {
        panic("bad alloc order %u\n", order);
        return NULL;
    }

    if (!prepare_alloc_pages(gfp_mask, order, &ac))
        return NULL;

    finalise_ac(gfp_mask, &ac);

    /* First allocation attempt */
    page = get_page_from_freelist(gfp_mask, order, alloc_flags, &ac);
    if (likely(page))
        return page;

    panic("alloc failed!\n");
    return NULL;
}
EXPORT_SYMBOL(__alloc_pages_nodemask);

/* Returns the next zone at or below highest_zoneidx in a zonelist */
struct zoneref *
__next_zones_zonelist(struct zoneref *z,
                      enum zone_type highest_zoneidx)
{
    /*
     * Find the next suitable zone to use for the allocation.
     * Only filter based on nodemask if it's set
     */
    while (zonelist_zone_idx(z) > highest_zoneidx)
        z++;

    return z;
}

static void zoneref_set_zone(struct zone *zone, struct zoneref *zoneref)
{
    zoneref->zone = zone;
    zoneref->zone_idx = zone_idx(zone);
}

/*
 * Builds allocation fallback zone lists.
 *
 * Add all populated zones of a node to the zonelist.
 */
static int
build_zonerefs_node(pg_data_t *pgdat, struct zoneref *zonerefs)
{
    struct zone *zone;
    enum zone_type zone_type = MAX_NR_ZONES;
    int nr_zones = 0;

    do {
        zone_type--;
        zone = pgdat->node_zones + zone_type;
        if (managed_zone(zone)) {
            zoneref_set_zone(zone, &zonerefs[nr_zones++]);
        }
    } while (zone_type);

    return nr_zones;
}

static void
build_zonelists(pg_data_t *pgdat)
{
    struct zoneref *zonerefs;
    int nr_zones;

    zonerefs = pgdat->node_zonelists[ZONELIST_FALLBACK]._zonerefs;
    nr_zones = build_zonerefs_node(pgdat, zonerefs);
    zonerefs += nr_zones;

    zonerefs->zone = NULL;
    zonerefs->zone_idx = 0;
}

static void
__build_all_zonelists(void)
{
    build_zonelists(NODE_DATA(0));
}

static void
pageset_update(struct per_cpu_pages *pcp,
               unsigned long high,
               unsigned long batch)
{
    /* Update high, then batch, in order */
    pcp->high = high;

    pcp->batch = batch;
}

/* a companion to pageset_set_high() */
static void
pageset_set_batch(struct per_cpu_pageset *p, unsigned long batch)
{
    pageset_update(&p->pcp, 6 * batch, max(1UL, 1 * batch));
}

static void
pageset_init(struct per_cpu_pageset *p)
{
    struct per_cpu_pages *pcp;

    memset(p, 0, sizeof(*p));

    pcp = &p->pcp;
    INIT_LIST_HEAD(&pcp->lists);
}

static void
setup_pageset(struct per_cpu_pageset *p, unsigned long batch)
{
    pageset_init(p);
    pageset_set_batch(p, batch);
}

static void
build_all_zonelists_init(void)
{
    __build_all_zonelists();
    setup_pageset(&boot_pageset, 0);
}

static void
reserve_first_chunk_for_cpu_cache(void)
{
    reserved_chunk_size = RESERVED_CHUNK_SIZE;
    reserved_chunk_ptr =
        memblock_alloc(reserved_chunk_size, PAGE_SIZE);
}

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;

    page = alloc_pages(gfp_mask & ~__GFP_HIGHMEM, order);
    if (!page)
        return 0;
    return (unsigned long) page_address(page);
}
EXPORT_SYMBOL(__get_free_pages);

void
free_pages(unsigned long addr, unsigned int order)
{
    if (addr != 0) {
        BUG_ON(!virt_addr_valid((void *)addr));
        __free_pages(virt_to_page((void *)addr), order);
    }
}
EXPORT_SYMBOL(free_pages);

static void *
make_alloc_exact(unsigned long addr, unsigned int order, size_t size)
{
    if (addr) {
        unsigned long alloc_end = addr + (PAGE_SIZE << order);
        unsigned long used = addr + PAGE_ALIGN(size);

        //split_page(virt_to_page((void *)addr), order);
        while (used < alloc_end) {
            free_page(used);
            used += PAGE_SIZE;
        }
    }
    return (void *)addr;
}

void *
alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
    unsigned long addr;
    unsigned int order = get_order(size);

    BUG_ON(gfp_mask & __GFP_COMP);

    addr = __get_free_pages(gfp_mask, order);
    return make_alloc_exact(addr, order, size);
}
EXPORT_SYMBOL(alloc_pages_exact);

/**
 * free_pages_exact - release memory allocated via alloc_pages_exact()
 * @virt: the value returned by alloc_pages_exact.
 * @size: size of allocation, same value as passed to alloc_pages_exact().
 *
 * Release the memory allocated by a previous call to alloc_pages_exact.
 */
void
free_pages_exact(void *virt, size_t size)
{
    unsigned long addr = (unsigned long)virt;
    unsigned long end = addr + PAGE_ALIGN(size);

    while (addr < end) {
        free_page(addr);
        addr += PAGE_SIZE;
    }
}
EXPORT_SYMBOL(free_pages_exact);

unsigned long get_zeroed_page(gfp_t gfp_mask)
{
    return __get_free_pages(gfp_mask | __GFP_ZERO, 0);
}
EXPORT_SYMBOL(get_zeroed_page);

static int
init_module(void)
{
    printk("module[buddy]: init begin ...\n");

    reserve_bootmem_region_fn = reserve_bootmem_region;
    free_pages_core_fn = __free_pages_core;

    max_low_pfn = PFN_DOWN(memblock_end_of_DRAM());
    set_max_mapnr(max_low_pfn);

    zone_sizes_init();
    build_all_zonelists_init();

    reserve_first_chunk_for_cpu_cache();
    memblock_free_all();

    printk("module[buddy]: init end!\n");
    return 0;
}

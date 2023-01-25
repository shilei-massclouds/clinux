/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MMZONE_H
#define _LINUX_MMZONE_H

#include <list.h>
#include <page.h>
#include <atomic.h>
#include <kernel.h>

#define MAX_ORDER 11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

#define MAX_NR_ZONES    3   /* __MAX_NR_ZONES */
#define ZONES_SHIFT     2
#define ZONES_WIDTH     ZONES_SHIFT

/* Maximum number of zones on a zonelist */
#define MAX_ZONES_PER_ZONELIST MAX_NR_ZONES

enum {
    ZONELIST_FALLBACK,  /* zonelist with fallback */
    MAX_ZONELISTS
};

/*
 * zone_idx() returns 0 for the ZONE_DMA zone,
 * 1 for the ZONE_NORMAL zone, etc.
 */
#define zone_idx(zone)  ((zone) - (zone)->zone_pgdat->node_zones)

enum zone_type {
    /*
     * ZONE_DMA and ZONE_DMA32 are used when there are peripherals not able
     * to DMA to all of the addressable memory (ZONE_NORMAL).
     * On architectures where this area covers the whole 32 bit address
     * space ZONE_DMA32 is used. ZONE_DMA is left for the ones with smaller
     * DMA addressing constraints. This distinction is important as a 32bit
     * DMA mask is assumed when ZONE_DMA32 is defined. Some 64-bit
     * platforms may need both zones as they support peripherals with
     * different DMA addressing limitations.
     *
     * ia64 and riscv only use ZONE_DMA32.
     */
    ZONE_DMA32,

    /*
     * Normal addressable memory is in ZONE_NORMAL. DMA operations can be
     * performed on pages in ZONE_NORMAL if the DMA devices support
     * transfers to all addressable memory.
     */
    ZONE_NORMAL,

    ZONE_MOVABLE,

    __MAX_NR_ZONES
};

enum zone_watermarks {
    WMARK_MIN,
    WMARK_LOW,
    WMARK_HIGH,
    NR_WMARK
};

#define for_next_zone_zonelist_nodemask(zone, z, zlist, highidx) \
    for (zone = z->zone; \
         zone; \
         z = next_zones_zonelist(++z, highidx), \
         zone = zonelist_zone(z))

#define for_each_order(order) \
    for (order = 0; order < MAX_ORDER; order++)

struct free_area {
    struct list_head    free_list;
    unsigned long       nr_free;
};

struct per_cpu_pages {
    int count;      /* number of pages in the list */
    int high;       /* high watermark, emptying needed */
    int batch;      /* chunk size for buddy add/remove */

    struct list_head lists;
};

struct per_cpu_pageset {
    struct per_cpu_pages pcp;
};

struct zone {
    struct pglist_data  *zone_pgdat;

    struct per_cpu_pageset *pageset;

    /* zone_start_pfn == zone_start_paddr >> PAGE_SHIFT */
    unsigned long   zone_start_pfn;

    atomic_long_t   managed_pages;
    unsigned long   spanned_pages;
    unsigned long   present_pages;

    const char      *name;

    int initialized;

    /* free areas of different sizes */
    struct free_area    free_area[MAX_ORDER];
};

/*
 * This struct contains information about a zone in a zonelist.
 * It is stored here to avoid dereferences into large structures and
 * lookups of tables
 */
struct zoneref {
    struct zone *zone;  /* Pointer to actual zone */
    int zone_idx;       /* zone_idx(zoneref->zone) */
};

/*
 * One allocation request operates on a zonelist. A zonelist
 * is a list of zones, the first one is the 'goal' of the
 * allocation, the other zones are fallback zones, in decreasing
 * priority.
 *
 * To speed the reading of the zonelist, the zonerefs contain the zone
 * index of the entry being read.
 * Helper functions to access information given a struct zoneref are
 * zonelist_zone()  - Return the struct zone * for an entry in _zonerefs
 * zonelist_zone_idx()  - Return the index of the zone for an entry
 * zonelist_node_idx()  - Return the index of the node for an entry
 */
struct zonelist {
    struct zoneref _zonerefs[MAX_ZONES_PER_ZONELIST + 1];
};


typedef struct pglist_data {
    /*
     * node_zones contains just the zones for THIS node. Not all of the
     * zones may be populated, but it is the full list. It is referenced by
     * this node's node_zonelists as well as other node's node_zonelists.
     */
    struct zone node_zones[MAX_NR_ZONES];

    /*
     * node_zonelists contains references to all zones in all nodes.
     * Generally the first zones will be references to this node's
     * node_zones.
     */
    struct zonelist node_zonelists[MAX_ZONELISTS];

    int nr_zones; /* number of populated zones in this node */

    enum zone_type kswapd_highest_zoneidx;

    unsigned long node_start_pfn;
    unsigned long node_present_pages; /* total number of physical pages */
    unsigned long node_spanned_pages; /* total size of physical page range,
                                         including holes */

    struct page *node_mem_map;
} pg_data_t;

static inline struct zone *zonelist_zone(struct zoneref *zoneref)
{
    return zoneref->zone;
}

static inline int zonelist_zone_idx(struct zoneref *zoneref)
{
    return zoneref->zone_idx;
}

struct zoneref *
__next_zones_zonelist(struct zoneref *z,
                      enum zone_type highest_zoneidx);

/**
 * next_zones_zonelist - Returns the next zone at or below highest_zoneidx within the allowed nodemask using a cursor within a zonelist as a starting point
 * @z - The cursor used as a starting point for the search
 * @highest_zoneidx - The zone index of the highest zone to return
 * @nodes - An optional nodemask to filter the zonelist with
 *
 * This function returns the next zone at or below a given zone index that is
 * within the allowed nodemask using a cursor as the starting point for the
 * search. The zoneref returned is a cursor that represents the current zone
 * being examined. It should be advanced by one before calling
 * next_zones_zonelist again.
 */
static __always_inline struct zoneref *
next_zones_zonelist(struct zoneref *z,
                    enum zone_type highest_zoneidx)
{
    if (likely(zonelist_zone_idx(z) <= highest_zoneidx))
        return z;
    return __next_zones_zonelist(z, highest_zoneidx);
}

/**
 * first_zones_zonelist - Returns the first zone at or below highest_zoneidx within the allowed nodemask in a zonelist
 * @zonelist - The zonelist to search for a suitable zone
 * @highest_zoneidx - The zone index of the highest zone to return
 * @nodes - An optional nodemask to filter the zonelist with
 * @return - Zoneref pointer for the first suitable zone found (see below)
 *
 * This function returns the first zone at or below a given zone index
 * that is within the allowed nodemask.
 * The zoneref returned is a cursor that can be used to iterate
 * the zonelist with next_zones_zonelist by advancing it by one
 * before calling.
 *
 * When no eligible zone is found, zoneref->zone is NULL
 * (zoneref itself is * never NULL).
 * This may happen either genuinely, or due to concurrent nodemask
 * update due to cpuset modification.
 */
static inline struct zoneref *
first_zones_zonelist(struct zonelist *zonelist,
                     enum zone_type highest_zoneidx)
{
    return next_zones_zonelist(zonelist->_zonerefs, highest_zoneidx);
}


extern struct pglist_data contig_page_data;
#define NODE_DATA(nid)  (&contig_page_data)

static inline unsigned long pgdat_end_pfn(pg_data_t *pgdat)
{
    return pgdat->node_start_pfn + pgdat->node_spanned_pages;
}

static inline struct page *
get_page_from_free_area(struct free_area *area)
{
    return list_first_entry_or_null(&area->free_list, struct page, lru);
}

static inline unsigned long
zone_managed_pages(struct zone *zone)
{
    return (unsigned long)atomic_long_read(&zone->managed_pages);
}

static inline bool
managed_zone(struct zone *zone)
{
    return zone_managed_pages(zone);
}

#endif /* _LINUX_MMZONE_H */

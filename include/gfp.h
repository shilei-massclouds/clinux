/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_GFP_H
#define __LINUX_GFP_H

#include <bug.h>
#include <mmzone.h>
#include <kernel.h>

#define ___GFP_DMA              0x01u
#define ___GFP_HIGHMEM          0x02u
#define ___GFP_DMA32            0x04u
#define ___GFP_MOVABLE          0x08u
#define ___GFP_RECLAIMABLE      0x10u
#define ___GFP_HIGH             0x20u
#define ___GFP_IO               0x40u
#define ___GFP_FS               0x80u
#define ___GFP_ZERO             0x100u
#define ___GFP_ATOMIC           0x200u
#define ___GFP_DIRECT_RECLAIM   0x400u
#define ___GFP_KSWAPD_RECLAIM   0x800u
#define ___GFP_WRITE            0x1000u
#define ___GFP_NOWARN           0x2000u
#define ___GFP_RETRY_MAYFAIL    0x4000u
#define ___GFP_NOFAIL           0x8000u
#define ___GFP_NORETRY          0x10000u
#define ___GFP_MEMALLOC         0x20000u
#define ___GFP_COMP             0x40000u
#define ___GFP_NOMEMALLOC       0x80000u
#define ___GFP_HARDWALL         0x100000u
#define ___GFP_ACCOUNT          0x400000u

/*
 * %__GFP_DIRECT_RECLAIM indicates that the caller may enter direct
 * reclaim. This flag can be cleared to avoid unnecessary delays when
 * a fallback option is available.
 *
 * %__GFP_KSWAPD_RECLAIM indicates that the caller wants to wake
 * kswapd when the low watermark is reached and have it reclaim pages
 * until the high watermark is reached. A caller may wish to clear this
 * flag when fallback options are available and the reclaim is likely
 * to disrupt the system. The canonical example is THP allocation
 * where a fallback is cheap but reclaim/compaction may cause indirect
 * stalls.
 *
 * %__GFP_RECLAIM is shorthand to allow/forbid both direct and
 * kswapd reclaim.
 */

#define __GFP_IO    ((gfp_t)___GFP_IO)
#define __GFP_FS    ((gfp_t)___GFP_FS)

#define __GFP_NOWARN    ((gfp_t)___GFP_NOWARN)
#define __GFP_COMP      ((gfp_t)___GFP_COMP)
#define __GFP_ZERO      ((gfp_t)___GFP_ZERO)

#define __GFP_NOFAIL        ((gfp_t)___GFP_NOFAIL)
#define __GFP_RETRY_MAYFAIL ((gfp_t)___GFP_RETRY_MAYFAIL)
#define __GFP_NORETRY       ((gfp_t)___GFP_NORETRY)

#define __GFP_RECLAIMABLE   ((gfp_t)___GFP_RECLAIMABLE)
#define __GFP_WRITE         ((__force gfp_t)___GFP_WRITE)
#define __GFP_HARDWALL      ((gfp_t)___GFP_HARDWALL)
#define __GFP_ACCOUNT       ((gfp_t)___GFP_ACCOUNT)

#define __GFP_DIRECT_RECLAIM    ((gfp_t)___GFP_DIRECT_RECLAIM) /* Caller can reclaim */
#define __GFP_KSWAPD_RECLAIM    ((gfp_t)___GFP_KSWAPD_RECLAIM) /* kswapd can wake */

#define __GFP_RECLAIM ((gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))

#define __GFP_ATOMIC        ((__force gfp_t)___GFP_ATOMIC)
#define __GFP_HIGH          ((__force gfp_t)___GFP_HIGH)
#define __GFP_MEMALLOC      ((__force gfp_t)___GFP_MEMALLOC)
#define __GFP_NOMEMALLOC    ((__force gfp_t)___GFP_NOMEMALLOC)

/*
 * %GFP_KERNEL is typical for kernel-internal allocations.
 * The caller requires %ZONE_NORMAL or a lower zone for direct access
 * but can direct reclaim.
 */
#define GFP_ATOMIC  (__GFP_HIGH|__GFP_ATOMIC|__GFP_KSWAPD_RECLAIM)
#define GFP_KERNEL  (__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define GFP_NOWAIT  (__GFP_KSWAPD_RECLAIM)
#define GFP_NOIO    (__GFP_RECLAIM)
#define GFP_NOFS    (__GFP_RECLAIM | __GFP_IO)

#define GFP_HIGHUSER            (GFP_USER | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE    (GFP_HIGHUSER | __GFP_MOVABLE)

#define GFP_KERNEL_ACCOUNT (GFP_KERNEL | __GFP_ACCOUNT)

struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order);

static inline struct page *
__alloc_pages(gfp_t gfp_mask, unsigned int order)
{
    return __alloc_pages_nodemask(gfp_mask, order);
}

#define alloc_pages(gfp_mask, order) __alloc_pages(gfp_mask, order)

#define alloc_page(gfp_mask) alloc_pages(gfp_mask, 0)

#define alloc_pages_vma(gfp_mask, order, vma, addr, node, false) \
    alloc_pages(gfp_mask, order)

#define alloc_page_vma(gfp_mask, vma, addr) \
    alloc_pages_vma(gfp_mask, 0, vma, addr, numa_node_id(), false)

void __free_pages(struct page *page, unsigned int order);

void free_pages(unsigned long addr, unsigned int order);

#define free_page(addr) free_pages((addr), 0)

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);

#define __get_free_page(gfp_mask) __get_free_pages((gfp_mask), 0)

void *alloc_pages_exact(size_t size, gfp_t gfp_mask);

void free_pages_exact(void *virt, size_t size);

/*
 * The set of flags that only affect watermark checking and reclaim
 * behaviour. This is used by the MM to obey the caller constraints
 * about IO, FS and watermark checking while ignoring placement
 * hints such as HIGHMEM usage.
 */
#define GFP_RECLAIM_MASK                            \
    (__GFP_RECLAIM|__GFP_HIGH|__GFP_IO|__GFP_FS|    \
     __GFP_NOWARN|__GFP_RETRY_MAYFAIL|__GFP_NOFAIL| \
     __GFP_NORETRY|__GFP_MEMALLOC|__GFP_NOMEMALLOC| \
     __GFP_ATOMIC)

#define __GFP_DMA       ((__force gfp_t)___GFP_DMA)
#define __GFP_HIGHMEM   ((__force gfp_t)___GFP_HIGHMEM)
#define __GFP_DMA32     ((__force gfp_t)___GFP_DMA32)
#define __GFP_MOVABLE   ((__force gfp_t)___GFP_MOVABLE)  /* ZONE_MOVABLE allowed */

#define GFP_USER \
    (__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_HARDWALL)

#define GFP_ZONEMASK \
    (__GFP_DMA|__GFP_HIGHMEM|__GFP_DMA32|__GFP_MOVABLE)

#define GFP_ZONE_TABLE ( \
    (ZONE_NORMAL << 0 * GFP_ZONES_SHIFT) | \
    (ZONE_NORMAL << ___GFP_DMA * GFP_ZONES_SHIFT) | \
    (ZONE_NORMAL << ___GFP_HIGHMEM * GFP_ZONES_SHIFT) | \
    (ZONE_DMA32  << ___GFP_DMA32 * GFP_ZONES_SHIFT) \
)

#define GFP_ZONES_SHIFT ZONES_SHIFT

static inline enum zone_type gfp_zone(gfp_t flags)
{
    enum zone_type z;
    int bit = (__force int) (flags & GFP_ZONEMASK);

    z = (GFP_ZONE_TABLE >> (bit * GFP_ZONES_SHIFT)) &
        ((1 << GFP_ZONES_SHIFT) - 1);
    return z;
}

static inline int gfp_zonelist(gfp_t flags)
{
    return ZONELIST_FALLBACK;
}

/*
 * We get the zone list from the current node and the gfp_mask.
 * This zone list contains a maximum of MAXNODES*MAX_NR_ZONES zones.
 * There are two zonelists per node, one for all zones with memory and
 * one containing just zones from the node the zonelist belongs to.
 *
 * For the normal case of non-DISCONTIGMEM systems the NODE_DATA() gets
 * optimized to &contig_page_data at compile-time.
 */
static inline struct zonelist *
node_zonelist(gfp_t flags)
{
    return NODE_DATA(0)->node_zonelists + gfp_zonelist(flags);
}

unsigned long get_zeroed_page(gfp_t gfp_mask);

#endif /* __LINUX_GFP_H */

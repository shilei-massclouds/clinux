// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <gfp.h>
#include <page.h>
#include <export.h>
#include <kernel.h>
#include <printk.h>
#include <hashtable.h>

#define ADAPT_SCALE_BASE    (64ul << 30)
#define ADAPT_SCALE_SHIFT   2
#define ADAPT_SCALE_NPAGES  (ADAPT_SCALE_BASE >> PAGE_SHIFT)

extern unsigned long nr_kernel_pages;

void *
alloc_large_system_hash(const char *tablename,
                        unsigned long bucketsize,
                        int scale,
                        unsigned int *_hash_shift,
                        unsigned int *_hash_mask)
{
    gfp_t gfp_flags;
    unsigned long adapt;
    unsigned long log2qty;
    unsigned long size;
    unsigned long long max;
    unsigned long numentries;
    void *table = NULL;

    /* round applicable memory size up to nearest megabyte */
    numentries = nr_kernel_pages;
    numentries = round_up(numentries, (1<<20)/PAGE_SIZE);

    for (adapt = ADAPT_SCALE_NPAGES; adapt < numentries;
         adapt <<= ADAPT_SCALE_SHIFT)
        scale++;

    /* limit to 1 bucket per 2^scale bytes of low memory */
    if (scale > PAGE_SHIFT)
        numentries >>= (scale - PAGE_SHIFT);
    else
        numentries <<= (PAGE_SHIFT - scale);

    numentries = roundup_pow_of_two(numentries);

    /* limit allocation size to 1/16 total memory by default */
    max = ((unsigned long long)nr_kernel_pages<< PAGE_SHIFT) >> 4;
    do_div(max, bucketsize);
    max = min(max, 0x80000000ULL);

    if (numentries > max)
        numentries = max;

    log2qty = ilog2(numentries);
    gfp_flags = __GFP_ZERO;
    do {
        size = bucketsize << log2qty;
        BUG_ON(get_order(size) >= MAX_ORDER);
        /*
         * If bucketsize is not a power-of-two, we may free
         * some pages at the end of hash table which
         * alloc_pages_exact() automatically does
         */
        table = alloc_pages_exact(size, gfp_flags);
    } while (!table && size > PAGE_SIZE && --log2qty);

    if (!table)
        panic("Failed to allocate %s hash table\n", tablename);

    printk("%s hash table entries: %ld (order: %d, %lu bytes, linear)\n",
           tablename, 1UL << log2qty, ilog2(size) - PAGE_SHIFT, size);

    if (_hash_shift)
        *_hash_shift = log2qty;
    if (_hash_mask)
        *_hash_mask = (1 << log2qty) - 1;

    return table;
}
EXPORT_SYMBOL(alloc_large_system_hash);

static int
init_module(void)
{
    printk("module[hashtable]: init begin ...\n");
    printk("module[hashtable]: init end!\n");
    return 0;
}

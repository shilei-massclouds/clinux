// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/xarray.h>
#include <linux/pgtable.h>
#include "../../booter/src/booter.h"

int
cl_vmalloc_init(void)
{
    sbi_puts("module[vmalloc]: init begin ...\n");
    sbi_puts("module[vmalloc]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_vmalloc_init);

int set_direct_map_invalid_noflush(struct page *page)
{
    booter_panic("No impl 'slub'.");
}

int __cond_resched_lock(spinlock_t *lock)
{
    booter_panic("No impl 'slub'.");
}

int set_direct_map_default_noflush(struct page *page)
{
    booter_panic("No impl 'slub'.");
}

void pmd_clear_bad(pmd_t *pmd)
{
    booter_panic("No impl 'slub'.");
}


int __printk_ratelimit(const char *func)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__printk_ratelimit);

void flush_icache_pte(pte_t pte)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(flush_icache_pte);
/*
void warn_alloc(gfp_t gfp_mask, nodemask_t *nodemask, const char *fmt, ...)
{
    booter_panic("No impl 'slub'.");
}
*/


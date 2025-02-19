// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/mmzone.h>
#include <linux/compaction.h>
#include <linux/oom.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_page_alloc_init(void)
{
    sbi_puts("module[page_alloc]: init begin ...\n");
    sbi_puts("module[page_alloc]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_page_alloc_init);

/*
void print_modules(void)
{
    booter_panic("No impl 'page_alloc'.");
}
*/

void lru_add_drain_cpu(int cpu)
{
    booter_panic("No impl 'page_alloc'.");
}

void __dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'page_alloc'.");
}

/*
unsigned long try_to_free_pages(struct zonelist *zonelist, int order,
                gfp_t gfp_mask, nodemask_t *nodemask)
{
    booter_panic("No impl 'page_alloc'.");
}
*/

void __kernel_map_pages(struct page *page, int numpages, int enable)
{
    booter_panic("No impl 'page_alloc'.");
}

bool compaction_zonelist_suitable(struct alloc_context *ac, int order,
        int alloc_flags)
{
    booter_panic("No impl 'page_alloc'.");
}

void compaction_defer_reset(struct zone *zone, int order,
        bool alloc_success)
{
    booter_panic("No impl 'page_alloc'.");
}

/*
void wakeup_kswapd(struct zone *zone, gfp_t gfp_flags, int order,
           enum zone_type highest_zoneidx)
{
    booter_panic("No impl 'page_alloc'.");
}

unsigned long zone_reclaimable_pages(struct zone *zone)
{
    booter_panic("No impl 'page_alloc'.");
}
*/

bool out_of_memory(struct oom_control *oc)
{
    booter_panic("No impl 'page_alloc'.");
}

enum compact_result try_to_compact_pages(gfp_t gfp_mask, unsigned int order,
        unsigned int alloc_flags, const struct alloc_context *ac,
        enum compact_priority prio, struct page **capture)
{
    booter_panic("No impl 'page_alloc'.");
}

/*
void show_mem(unsigned int filter, nodemask_t *nodemask)
{
    booter_panic("No impl 'page_init_poison'.");
}
*/

long congestion_wait(int sync, long timeout)
{
    booter_panic("No impl 'page_init_poison'.");
}
EXPORT_SYMBOL(congestion_wait);

bool node_dirty_ok(struct pglist_data *pgdat)
{
    booter_panic("No impl 'page_init_poison'.");
}

/*
 * Serializes oom killer invocations (out_of_memory()) from all contexts to
 * prevent from over eager oom killing (e.g. when the oom killer is invoked
 * from different domains).
 *
 * oom_killer_disable() relies on this lock to stabilize oom_killer_disabled
 * and mark_oom_victim
 */
DEFINE_MUTEX(oom_lock);

/*
void show_swap_cache_info(void)
{
    booter_panic("No impl.");
}
*/

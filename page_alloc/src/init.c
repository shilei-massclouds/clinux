// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/workqueue.h>
#include <linux/mmzone.h>
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
 * Manage combined zone based / global counters
 *
 * vm_stat contains the global counters
 */
atomic_long_t vm_zone_stat[NR_VM_ZONE_STAT_ITEMS] __cacheline_aligned_in_smp;
atomic_long_t vm_numa_stat[NR_VM_NUMA_STAT_ITEMS] __cacheline_aligned_in_smp;
atomic_long_t vm_node_stat[NR_VM_NODE_STAT_ITEMS] __cacheline_aligned_in_smp;
EXPORT_SYMBOL(vm_zone_stat);
EXPORT_SYMBOL(vm_numa_stat);
EXPORT_SYMBOL(vm_node_stat);

void vm_events_fold_cpu(int cpu)
{
    booter_panic("No impl 'page_alloc'.");
}

void print_modules(void)
{
    booter_panic("No impl 'page_alloc'.");
}

void lru_add_drain_cpu(int cpu)
{
    booter_panic("No impl 'page_alloc'.");
}

bool queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
               struct delayed_work *dwork, unsigned long delay)
{
    booter_panic("No impl 'page_alloc'.");
}

void __dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'page_alloc'.");
}

struct workqueue_struct *system_wq __read_mostly;
EXPORT_SYMBOL(system_wq);

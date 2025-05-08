// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <asm/pgtable.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_paging_init(void)
{
    sbi_puts("module[top_paging]: init begin ...\n");
    REQUIRE_COMPONENT(early_printk);
    cl_init();

    extern unsigned long nr_all_pages;
    printk("nr_all_pages: %ld\n", nr_all_pages);

    sbi_puts("module[top_paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_paging_init);

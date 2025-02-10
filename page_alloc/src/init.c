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

void print_modules(void)
{
    booter_panic("No impl 'page_alloc'.");
}

void lru_add_drain_cpu(int cpu)
{
    booter_panic("No impl 'page_alloc'.");
}

void __dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'page_alloc'.");
}

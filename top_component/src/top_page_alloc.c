// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/gfp.h>
#include <linux/memblock.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_top_page_alloc_init(void)
{
    unsigned long nr_pages;
    struct page *page;

    sbi_puts("module[top_page_alloc]: init begin ...\n");
    REQUIRE_COMPONENT(early_printk);
    cl_init();

    nr_pages = memblock_free_all();
    printk("nr_pages: %lu\n", nr_pages);

    page = alloc_pages(GFP_KERNEL, 0);
    if (page == NULL) {
        printk("alloc pages error!\n");
    }
    __free_page(page);

    sbi_puts("module[top_page_alloc]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_page_alloc_init);

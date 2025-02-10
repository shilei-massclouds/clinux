// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "internal.h"
#include "../../booter/src/booter.h"

unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)]
                            __page_aligned_bss;
EXPORT_SYMBOL(empty_zero_page);

int
cl_paging_init(void)
{
    sbi_puts("module[paging]: init begin ...\n");
    sbi_puts("module[paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_paging_init);

void dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'dump_page'.");
}
EXPORT_SYMBOL(dump_page);

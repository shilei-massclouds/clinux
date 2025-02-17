// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/mm.h>
#include "../../booter/src/booter.h"

int
cl_show_mem_init(void)
{
    sbi_puts("module[show_mem]: init begin ...\n");
    sbi_puts("module[show_mem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_show_mem_init);

__weak void show_free_areas(unsigned int filter, nodemask_t *nodemask)
{
    booter_panic("No impl.");
}

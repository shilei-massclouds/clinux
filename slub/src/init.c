// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/smp.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include "../../booter/src/booter.h"

int
cl_slub_init(void)
{
    sbi_puts("module[slub]: init begin ...\n");
    kmem_cache_init();
    sbi_puts("module[slub]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_slub_init);

unsigned int stack_trace_save(unsigned long *store, unsigned int size,
                  unsigned int skipnr)
{
    booter_panic("No impl 'slub'.");
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

extern void setup_vm_final(void);

int
cl_paging_init(void)
{
    sbi_puts("module[paging]: init begin ...\n");
    sbi_puts("module[paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_paging_init);

void __init paging_init(void)
{
    setup_vm_final();
    //sparse_init();
    //setup_zero_page();
    //zone_sizes_init();
    //resource_init();
    booter_panic("In-Process 'paging_init'.");
}
EXPORT_SYMBOL(paging_init);

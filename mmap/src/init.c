// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <asm/page.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_mmap_init(void)
{
    sbi_puts("module[mmap]: init begin ...\n");
    sbi_puts("module[mmap]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_mmap_init);

DEFINE_ENABLE_FUNC(mmap);

/* enforced gap between the expanding stack and other mappings. */
unsigned long stack_guard_gap = 256UL<<PAGE_SHIFT;


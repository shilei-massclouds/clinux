// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

int
cl_vdso_init(void)
{
    sbi_puts("module[vdso]: init begin ...\n");
    sbi_puts("module[vdso]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_vdso_init);

extern char __vdso_rt_sigreturn[];
EXPORT_SYMBOL(__vdso_rt_sigreturn);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cred.h>
#include "../../booter/src/booter.h"

int
cl_cred_init(void)
{
    sbi_puts("module[cred]: init begin ...\n");
    sbi_puts("module[cred]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_cred_init);

int suid_dumpable = 0;
EXPORT_SYMBOL(suid_dumpable);

void groups_free(struct group_info *group_info)
{
    booter_panic("No impl.\n");
}

__weak void set_dumpable(struct mm_struct *mm, int value)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(set_dumpable);

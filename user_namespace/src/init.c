// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/highuid.h>
#include <linux/cred.h>
#include <linux/user_namespace.h>
#include "../../booter/src/booter.h"

int
cl_user_namespace_init(void)
{
    sbi_puts("module[user_namespace]: init begin ...\n");
    sbi_puts("module[user_namespace]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_user_namespace_init);

int overflowuid = DEFAULT_OVERFLOWUID;
int overflowgid = DEFAULT_OVERFLOWGID;


void *memdup_user_nul(const void __user *src, size_t len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(memdup_user_nul);

bool current_chrooted(void)
{
    booter_panic("No impl!\n");
}


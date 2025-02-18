// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/umh.h>
#include <linux/cred.h>
#include <linux/refcount.h>
#include "../../booter/src/booter.h"

int
cl_keys_init(void)
{
    sbi_puts("module[keys]: init begin ...\n");
    sbi_puts("module[keys]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_keys_init);

void kfree_sensitive(const void *p)
{
    booter_panic("No impl!\n");
}

int groups_search(const struct group_info *group_info, kgid_t grp)
{
    booter_panic("No impl!\n");
}

void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kmemdup);

bool refcount_dec_and_lock(refcount_t *r, spinlock_t *lock)
{
    booter_panic("No impl!\n");
}

//
// Consider to add a standalone component 'cred_itf'.
// Move functions below to it.
//

__weak void abort_creds(struct cred *new)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(abort_creds);

__weak void __put_cred(struct cred *cred)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__put_cred);

__weak struct cred *prepare_creds(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(prepare_creds);

int __weak commit_creds(struct cred *new)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(commit_creds);

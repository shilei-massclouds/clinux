// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_dcache_init(void)
{
    sbi_puts("module[dcache]: init begin ...\n");
    sbi_puts("module[dcache]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_dcache_init);

/*
bool list_lru_del(struct list_lru *lru, struct list_head *item)
{
    booter_panic("No impl!\n");
}
void __fsnotify_inode_delete(struct inode *inode)
{
    booter_panic("No impl!\n");
}
*/
int lockref_put_or_lock(struct lockref *lockref)
{
    booter_panic("No impl!\n");
}
void lockref_mark_dead(struct lockref *lockref)
{
    booter_panic("No impl!\n");
}
int lockref_put_return(struct lockref *lockref)
{
    booter_panic("No impl!\n");
}
/*
bool list_lru_add(struct list_lru *lru, struct list_head *item)
{
    booter_panic("No impl!\n");
}
*/

__weak void __init mnt_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(mnt_init);

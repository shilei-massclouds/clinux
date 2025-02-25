// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_inode_init(void)
{
    sbi_puts("module[inode]: init begin ...\n");
    sbi_puts("module[inode]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_inode_init);

const struct file_operations pipefifo_fops;

int __break_lease(struct inode *inode, unsigned int mode, unsigned int type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__break_lease);

/*
void __fsnotify_inode_delete(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__fsnotify_inode_delete);
*/

unsigned long invalidate_mapping_pages(struct address_space *mapping,
        pgoff_t start, pgoff_t end)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(invalidate_mapping_pages);

int __mnt_want_write_file(struct file *file)
{
    booter_panic("No impl!\n");
}

/*
*/

void truncate_inode_pages_final(struct address_space *mapping)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(truncate_inode_pages_final);

/*
int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock)
{
    booter_panic("No impl!\n");
}
*/
__weak void inode_io_list_del(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inode_io_list_del);

__weak void __mark_inode_dirty(struct inode *inode, int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__mark_inode_dirty);

int inode_has_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}

const struct file_operations def_blk_fops;

void bd_forget(struct inode *inode)
{
    booter_panic("No impl!\n");
}
/*
void list_lru_isolate_move(struct list_lru_one *list, struct list_head *item,
               struct list_head *head)
{
    booter_panic("No impl!\n");
}
*/
void
locks_free_lock_context(struct inode *inode)
{
    booter_panic("No impl!\n");
}
int remove_inode_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}
/*
unsigned long
list_lru_walk_one(struct list_lru *lru, int nid, struct mem_cgroup *memcg,
          list_lru_walk_cb isolate, void *cb_arg,
          unsigned long *nr_to_walk)
{
    booter_panic("No impl!\n");
}
*/

int sysctl_vfs_cache_pressure __read_mostly = 100;

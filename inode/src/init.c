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

void __fsnotify_inode_delete(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__fsnotify_inode_delete);

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

int fsnotify(__u32 mask, const void *data, int data_type, struct inode *dir,
         const struct qstr *file_name, struct inode *inode, u32 cookie)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fsnotify);

int __mnt_want_write(struct vfsmount *m)
{
    booter_panic("No impl!\n");
}

int in_group_p(kgid_t grp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(in_group_p);

void truncate_inode_pages_final(struct address_space *mapping)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(truncate_inode_pages_final);

/*
bool capable_wrt_inode_uidgid(const struct inode *inode, int cap)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(capable_wrt_inode_uidgid);
*/

int _atomic_dec_and_lock(atomic_t *atomic, spinlock_t *lock)
{
    booter_panic("No impl!\n");
}
/*
void list_lru_isolate(struct list_lru_one *list, struct list_head *item)
{
    booter_panic("No impl!\n");
}
int
send_sig(int sig, struct task_struct *p, int priv)
{
    booter_panic("No impl!\n");
}
*/
void inode_io_list_del(struct inode *inode)
{
    booter_panic("No impl!\n");
}
void __mnt_drop_write_file(struct file *file)
{
    booter_panic("No impl!\n");
}

void __mark_inode_dirty(struct inode *inode, int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__mark_inode_dirty);

int inode_has_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}

const struct file_operations def_blk_fops;

int cap_inode_need_killpriv(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
void bd_forget(struct inode *inode)
{
    booter_panic("No impl!\n");
}
void inode_wait_for_writeback(struct inode *inode)
{
    booter_panic("No impl!\n");
}
int write_inode_now(struct inode *inode, int sync)
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

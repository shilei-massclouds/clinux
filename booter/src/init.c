// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/cache.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/user_namespace.h>
#include <asm/sbi.h>
#include <asm/current.h>
#include <cl_hook.h>
#include "booter.h"

#define UL_STR_SIZE 19  /* prefix with '0x' and end with '\0' */

__visible u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

extern int cl_init();

void
start_kernel(void)
{
    cl_init();
    booter_panic("Kernel has been terminated normally!");
}

void sbi_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            sbi_console_putchar('\r');
        sbi_console_putchar(*s);
    }
}
EXPORT_SYMBOL(sbi_puts);

void sbi_put_u64(unsigned long n)
{
    char buf[UL_STR_SIZE];
    hex_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_u64);

void sbi_put_dec(unsigned long n)
{
    char buf[UL_STR_SIZE];
    dec_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_dec);

//
// NOTE ************************
// Remove below definitions in future.
//

/**
 *  panic - halt the system
 *  @fmt: The text string to print
 *
 *  Display a message, then perform cleanups.
 *
 *  This function never returns.
 */
void __weak panic(const char *fmt, ...)
{
    sbi_puts("[RAW_PANIC] ");
    sbi_puts(fmt);
    sbi_puts("\n");
    sbi_shutdown();
    do {} while (1);
}
EXPORT_SYMBOL(panic);

__weak void __warn_printk(const char *fmt, ...)
{
    sbi_puts("[RAW_WARN_PRINTK] ");
    sbi_puts(fmt);
    sbi_puts("\n");
    sbi_shutdown();
}
EXPORT_SYMBOL(__warn_printk);

__weak void sysfs_remove_link(struct kobject *kobj, const char *name)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(sysfs_remove_link);

__weak void sysfs_delete_link(struct kobject *kobj, struct kobject *targ,
			const char *name)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(sysfs_delete_link);

int kobject_uevent(struct kobject *kobj, enum kobject_action action)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobject_uevent);

void dev_printk(const char *level, const struct device *dev,
        const char *fmt, ...)
{
    sbi_puts("[RAW_DEV_PRINTK]:");
    sbi_puts(level);
    sbi_puts(" - ");
    sbi_puts(fmt);
    sbi_puts("\n");
}
EXPORT_SYMBOL(dev_printk);

unsigned long lpj_fine;
EXPORT_SYMBOL(lpj_fine);

void __local_bh_enable_ip(unsigned long ip, unsigned int cnt)
{
    booter_panic("No impl '__local_bh_enable_ip'.");
}
EXPORT_SYMBOL(__local_bh_enable_ip);

#ifndef __ARCH_IRQ_STAT
DEFINE_PER_CPU_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);
#endif

void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_data);

int kthread_stop(struct task_struct *k)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_stop);

struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data, int node,
                       const char namefmt[],
                       ...)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_create_on_node);

void __put_task_struct(struct task_struct *tsk)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(__put_task_struct);

struct kobject *kernel_kobj;
EXPORT_SYMBOL_GPL(kernel_kobj);

__weak void module_put(struct module *module)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(module_put);

__weak int sysfs_create_link(struct kobject *kobj, struct kobject *target,
              const char *name)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(sysfs_create_link);

const struct sysfs_ops kobj_sysfs_ops;
EXPORT_SYMBOL(kobj_sysfs_ops);

#ifdef CONFIG_STACKPROTECTOR
#include <linux/stackprotector.h>
unsigned long __stack_chk_guard __read_mostly;
EXPORT_SYMBOL(__stack_chk_guard);
#endif

bool capable(int cap)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(capable);

int proc_dointvec(struct ctl_table *table, int write, void *buffer,
          size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(proc_dointvec);

bool __weak irq_work_needs_cpu(void)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(irq_work_needs_cpu);

void __weak irq_work_tick(void)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(irq_work_tick);

void kill_fasync(struct fasync_struct **fp, int sig, int band)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kill_fasync);

int fasync_helper(int fd, struct file * filp, int on, struct fasync_struct **fapp)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(fasync_helper);

__weak void proc_free_inum(unsigned int inum)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_free_inum);

__weak int proc_alloc_inum(unsigned int *inum)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_alloc_inum);

__weak void *__vmalloc(unsigned long size, gfp_t gfp_mask)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(__vmalloc);

__weak void show_mem(unsigned int filter, nodemask_t *nodemask)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(show_mem);

int panic_on_warn __read_mostly;
EXPORT_SYMBOL(panic_on_warn);

void show_state_filter(unsigned long state_filter)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(show_state_filter);

__weak void kernfs_get(struct kernfs_node *kn)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL_GPL(kernfs_get);

__weak void kernfs_put(struct kernfs_node *kn)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL(kernfs_put);

__weak void kill_anon_super(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kill_anon_super);

__weak int simple_readpage(struct file *file, struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_readpage);

__weak int simple_write_begin(struct file *file, struct address_space *mapping,
            loff_t pos, unsigned len, unsigned flags,
            struct page **pagep, void **fsdata)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_write_begin);

__weak int simple_write_end(struct file *file, struct address_space *mapping,
            loff_t pos, unsigned len, unsigned copied,
            struct page *page, void *fsdata)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_write_end);

/*
int register_filesystem(struct file_system_type * fs)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(register_filesystem);
*/

__weak struct module *__module_address(unsigned long addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__module_address);

__weak bool try_module_get(struct module *module)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(try_module_get);

int call_usermodehelper_exec(struct subprocess_info *sub_info, int wait)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(call_usermodehelper_exec);

struct subprocess_info *call_usermodehelper_setup(const char *path, char **argv,
        char **envp, gfp_t gfp_mask,
        int (*init)(struct subprocess_info *info, struct cred *new),
        void (*cleanup)(struct subprocess_info *info),
        void *data)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(call_usermodehelper_setup);

__weak struct proc_dir_entry *proc_create_single_data(const char *name, umode_t mode,
        struct proc_dir_entry *parent,
        int (*show)(struct seq_file *, void *), void *data)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_create_single_data);

__weak int sysfs_create_file_ns(struct kobject *kobj, const struct attribute *attr,
             const void *ns)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(sysfs_create_file_ns);

__weak int sysfs_create_dir_ns(struct kobject *kobj, const void *ns)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sysfs_create_dir_ns);

__weak void sysfs_remove_dir(struct kobject *kobj)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sysfs_remove_dir);

__weak void logfc(struct fc_log *log, const char *prefix, char level, const char *fmt, ...)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(logfc);

__weak void deactivate_locked_super(struct super_block *s)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(deactivate_locked_super);

__weak void put_fs_context(struct fs_context *fc)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_fs_context);

__weak int parse_monolithic_mount_data(struct fs_context *fc, void *data)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(parse_monolithic_mount_data);

__weak struct fs_context *fs_context_for_reconfigure(struct dentry *dentry,
					unsigned int sb_flags,
					unsigned int sb_flags_mask)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fs_context_for_reconfigure);

__weak void dput(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(dput);

int rcuwait_wake_up(struct rcuwait *w)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(rcuwait_wake_up);

__weak void free_prealloced_shrinker(struct shrinker *shrinker)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(free_prealloced_shrinker);

__weak int prealloc_shrinker(struct shrinker *shrinker)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(prealloc_shrinker);

__weak unsigned long try_to_free_pages(struct zonelist *zonelist, int order,
				gfp_t gfp_mask, nodemask_t *nodemask)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(try_to_free_pages);

__weak void register_shrinker_prepared(struct shrinker *shrinker)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(register_shrinker_prepared);

__weak void unregister_shrinker(struct shrinker *shrinker)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(unregister_shrinker);

__weak void wakeup_kswapd(struct zone *zone, gfp_t gfp_flags, int order,
		   enum zone_type highest_zoneidx)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(wakeup_kswapd);

__weak unsigned long zone_reclaimable_pages(struct zone *zone)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(zone_reclaimable_pages);

void __put_page(struct page *page)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(__put_page);

void unlock_page(struct page *page)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(unlock_page);

void __lock_page(struct page *__page)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(__lock_page);

__weak void prandom_bytes(void *buf, size_t bytes)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(prandom_bytes);

__weak void get_random_bytes(void *buf, int nbytes)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(get_random_bytes);

__weak struct vfsmount *kern_mount(struct file_system_type *type)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL_GPL(kern_mount);

__weak void put_filesystem(struct file_system_type *fs)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(put_filesystem);

__weak struct file_system_type *get_filesystem(struct file_system_type *fs)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(get_filesystem);

int sync_filesystem(struct super_block *sb)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(sync_filesystem);

__weak void path_get(const struct path *path)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(path_get);

__weak void path_put(const struct path *path)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(path_put);

/*
 * vfsmount lock may be taken for read to prevent changes to the
 * vfsmount hash, ie. during mountpoint lookups or walking back
 * up the tree.
 *
 * It should be taken for write in all cases where the vfsmount
 * tree or hash is modified or when a vfsmount structure is modified.
 */
__cacheline_aligned_in_smp DEFINE_SEQLOCK(mount_lock);
EXPORT_SYMBOL_GPL(mount_lock);

__weak void __detach_mounts(struct dentry *dentry)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(__detach_mounts);

struct page *pagecache_get_page(struct address_space *mapping, pgoff_t index,
        int fgp_flags, gfp_t gfp_mask)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(pagecache_get_page);

__weak struct vfsmount *mntget(struct vfsmount *mnt)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(mntget);

__weak void mntput(struct vfsmount *mnt)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(mntput);

int
task_work_add(struct task_struct *task, struct callback_head *work, int notify)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(task_work_add);

__weak
struct vfsmount *vfs_kern_mount(struct file_system_type *type,
				int flags, const char *name,
				void *data)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(vfs_kern_mount);

__weak int noop_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(noop_fsync);

__weak int simple_statfs(struct dentry *dentry, struct kstatfs *buf)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_statfs);

__weak void make_empty_dir_inode(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(make_empty_dir_inode);

__weak void d_genocide(struct dentry *parent)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(d_genocide);

__weak ssize_t generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_read_dir);

__weak void kfree_link(void *p)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kfree_link);

int nonseekable_open(struct inode *inode, struct file *filp)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(nonseekable_open);

loff_t
generic_file_llseek_size(struct file *file, loff_t offset, int whence,
        loff_t maxsize, loff_t eof)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(generic_file_llseek_size);

int set_page_dirty(struct page *page)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(set_page_dirty);

loff_t noop_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(noop_llseek);

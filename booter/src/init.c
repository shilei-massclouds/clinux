// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/cache.h>
#include <linux/jiffies.h>
#include <linux/swap.h>
#include <linux/kobject.h>
#include <linux/netlink.h>
#include <linux/syscore_ops.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/ipc_namespace.h>
#include <linux/user_namespace.h>
#include <linux/kobj_map.h>
#include <asm/sbi.h>
#include <asm/current.h>
#include <cl_hook.h>
#include "booter.h"
#include "../fs/internal.h"

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

__weak void dev_printk(const char *level, const struct device *dev,
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

__weak void __local_bh_enable_ip(unsigned long ip, unsigned int cnt)
{
    booter_panic("No impl '__local_bh_enable_ip'.");
}
EXPORT_SYMBOL(__local_bh_enable_ip);

#ifndef __ARCH_IRQ_STAT
DEFINE_PER_CPU_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);
#endif

__weak void *kthread_data(struct task_struct *task)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(kthread_data);

__weak int kthread_stop(struct task_struct *k)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_stop);

__weak struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data, int node,
                       const char namefmt[],
                       ...)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_create_on_node);

__weak void __put_task_struct(struct task_struct *tsk)
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

__weak bool capable(int cap)
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

__weak struct proc_dir_entry *proc_symlink(const char *name,
		struct proc_dir_entry *parent, const char *dest)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(proc_symlink);

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

__weak int call_usermodehelper_exec(struct subprocess_info *sub_info, int wait)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(call_usermodehelper_exec);

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

__weak void sysfs_remove_file_ns(struct kobject *kobj, const struct attribute *attr,
			  const void *ns)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(sysfs_remove_file_ns);

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

__weak int rcuwait_wake_up(struct rcuwait *w)
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

__weak void __put_page(struct page *page)
{
    booter_panic("No impl in 'booter'.");
}
EXPORT_SYMBOL(__put_page);

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

loff_t
generic_file_llseek_size(struct file *file, loff_t offset, int whence,
        loff_t maxsize, loff_t eof)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(generic_file_llseek_size);

loff_t noop_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(noop_llseek);

__weak int setattr_prepare(struct dentry *dentry, struct iattr *attr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(setattr_prepare);

void generic_fillattr(struct inode *inode, struct kstat *stat)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_fillattr);

/*
void setattr_copy(struct inode *inode, const struct iattr *attr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(setattr_copy);
*/

__weak struct pseudo_fs_context *init_pseudo(struct fs_context *fc,
					unsigned long magic)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(init_pseudo);

__weak void mark_page_accessed(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(mark_page_accessed);

struct pagevec;
__weak void __pagevec_release(struct pagevec *pvec)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__pagevec_release);

__weak bool shmem_mapping(struct address_space *mapping)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shmem_mapping);

errseq_t errseq_set(errseq_t *eseq, int err)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(errseq_set);

int try_to_free_buffers(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(try_to_free_buffers);

__weak int send_sig(int sig, struct task_struct *p, int priv)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(send_sig);

#include <linux/backing-dev-defs.h>
void wakeup_flusher_threads(enum wb_reason reason)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wakeup_flusher_threads);

void blk_finish_plug(struct blk_plug *plug)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_finish_plug);

void blk_start_plug(struct blk_plug *plug)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_start_plug);

__weak bool node_dirty_ok(struct pglist_data *pgdat)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(node_dirty_ok);

int proc_doulongvec_minmax(struct ctl_table *table, int write,
               void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_doulongvec_minmax);

__weak const void *kernfs_super_ns(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kernfs_super_ns);

__weak void kernfs_kill_sb(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kernfs_kill_sb);

/*
unsigned long __fdget(unsigned int fd)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__fdget);
*/

void free_uid(struct user_struct *up)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(free_uid);

__weak struct sighand_struct *__lock_task_sighand(struct task_struct *tsk,
					   unsigned long *flags)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__lock_task_sighand);

__weak int group_send_sig_info(int sig, struct kernel_siginfo *info,
			struct task_struct *p, enum pid_type type)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(group_send_sig_info);

__weak int __group_send_sig_info(int sig, struct kernel_siginfo *info, struct task_struct *p)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(__group_send_sig_info);

__weak int kill_pid(struct pid *pid, int sig, int priv)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kill_pid);

__weak struct pid *pidfd_pid(const struct file *file)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(pidfd_pid);

__weak int kill_pgrp(struct pid *pid, int sig, int priv)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kill_pgrp);

__weak int is_current_pgrp_orphaned(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(is_current_pgrp_orphaned);

char *d_path(const struct path *path, char *buf, int buflen)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(d_path);

__weak void __set_task_comm(struct task_struct *tsk, const char *buf, bool exec)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(__set_task_comm);

__weak bool ns_capable(struct user_namespace *ns, int cap)
{
    booter_panic("No impl 'ns_capable'.");
}
EXPORT_SYMBOL(ns_capable);

__weak unsigned int full_name_hash(const void *salt, const char *name, unsigned int len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(full_name_hash);

loff_t no_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(no_llseek);

__weak u64 hashlen_string(const void *salt, const char *name)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(hashlen_string);

__weak void proc_tty_register_driver(struct tty_driver *driver)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_tty_register_driver);
__weak void proc_tty_unregister_driver(struct tty_driver *driver)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_tty_unregister_driver);

__weak struct ctl_table_header *register_sysctl(const char *path, struct ctl_table *table)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(register_sysctl);

__weak void setup_sysctl_set(struct ctl_table_set *set,
	struct ctl_table_root *root,
	int (*is_seen)(struct ctl_table_set *))
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(setup_sysctl_set);

__weak void retire_sysctl_set(struct ctl_table_set *set)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(retire_sysctl_set);

__weak void unregister_sysctl_table(struct ctl_table_header * header)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(unregister_sysctl_table);

__weak struct ctl_table_header *__register_sysctl_table(
	struct ctl_table_set *set,
	const char *path, struct ctl_table *table)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__register_sysctl_table);

// From fs/proc/proc_sysctl.c
/* shared constants to be used in various sysctls */
const int sysctl_vals[] = { 0, 1, INT_MAX };
EXPORT_SYMBOL(sysctl_vals);

__weak struct ctl_table_header *register_sysctl_table(struct ctl_table *table)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(register_sysctl_table);

__weak int proc_cgroup_show(struct seq_file *m, struct pid_namespace *ns,
		     struct pid *pid, struct task_struct *tsk)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_cgroup_show);

bool refcount_dec_not_one(refcount_t *r)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(refcount_dec_not_one);

bool cancel_work_sync(struct work_struct *work)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cancel_work_sync);

__weak u64 get_random_u64(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(get_random_u64);

__weak void recalc_sigpending(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(recalc_sigpending);

void cgroup_enter_frozen(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cgroup_enter_frozen);

__weak void ptrace_notify(int exit_code)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(ptrace_notify);

__weak void task_join_group_stop(struct task_struct *task)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(task_join_group_stop);

__weak void
flush_signal_handlers(struct task_struct *t, int force_default)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(flush_signal_handlers);

__weak bool task_set_jobctl_pending(struct task_struct *task, unsigned long mask)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(task_set_jobctl_pending);

void cgroup_leave_frozen(bool always_leave)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cgroup_leave_frozen);

/*
int cap_vm_enough_memory(struct mm_struct *mm, long pages)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cap_vm_enough_memory);
*/

__weak void __noreturn do_exit(long code)
{
    booter_panic("No impl!\n");
    do {} while(1);
}
EXPORT_SYMBOL(do_exit);

__weak void ignore_signals(struct task_struct *t)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(ignore_signals);

__weak int tsk_fork_get_node(struct task_struct *tsk)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(tsk_fork_get_node);

__weak void kthread_bind_mask(struct task_struct *p, const struct cpumask *mask)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kthread_bind_mask);

__weak void *kthread_probe_data(struct task_struct *task)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(kthread_probe_data);

__weak void free_kthread_struct(struct task_struct *k)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(free_kthread_struct);

/*
__weak pid_t __task_pid_nr_ns(struct task_struct *task, enum pid_type type,
			struct pid_namespace *ns)
{
    booter_panic("No impl in 'workqueue'.");
}
*/
__weak void cgroup_cancel_fork(struct task_struct *child,
			struct kernel_clone_args *kargs)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(cgroup_cancel_fork);

__weak void fput(struct file *file)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL_GPL(fput);

__weak int cgroup_can_fork(struct task_struct *child, struct kernel_clone_args *kargs)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(cgroup_can_fork);

__weak void cgroup_post_fork(struct task_struct *child,
		      struct kernel_clone_args *kargs)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(cgroup_post_fork);

__weak void cgroup_fork(struct task_struct *child)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(cgroup_fork);

__weak int unshare_fd(unsigned long unshare_flags, unsigned int max_fds,
	       struct files_struct **new_fdp)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(unshare_fd);

__weak void __mnt_drop_write(struct vfsmount *mnt)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(__mnt_drop_write);

void exit_io_context(struct task_struct *task)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(exit_io_context);

void exit_shm(struct task_struct *task)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(exit_shm);

__weak void do_group_exit(int exit_code)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(do_group_exit);

__weak void __wake_up_parent(struct task_struct *p, struct task_struct *parent)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__wake_up_parent);

void task_work_run(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(task_work_run);

__weak void put_task_struct_rcu_user(struct task_struct *task)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(put_task_struct_rcu_user);

__weak bool thread_group_exited(struct pid *pid)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(thread_group_exited);

void exit_itimers(struct signal_struct *sig)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(exit_itimers);

void vfree(const void *addr)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(vfree);

__weak char *__get_task_comm(char *buf, size_t buf_size, struct task_struct *tsk)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(__get_task_comm);

__weak void lru_add_drain(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(lru_add_drain);

__weak bool path_noexec(const struct path *path)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(path_noexec);

struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL_GPL(find_vma);

/*
struct file *file_open_root(struct dentry *dentry, struct vfsmount *mnt,
                const char *filename, int flags, umode_t mode)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(file_open_root);
*/

__weak void global_dirty_limits(unsigned long *pbackground, unsigned long *pdirty)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(global_dirty_limits);

__weak int lookup_symbol_name(unsigned long addr, char *symname)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(lookup_symbol_name);

__weak notrace void touch_softlockup_watchdog(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(touch_softlockup_watchdog);

__weak notrace void touch_softlockup_watchdog_sched(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(touch_softlockup_watchdog_sched);

__weak void touch_all_softlockup_watchdogs(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(touch_all_softlockup_watchdogs);

int initrd_below_start_ok;
EXPORT_SYMBOL(initrd_below_start_ok);
phys_addr_t phys_initrd_start __initdata;
EXPORT_SYMBOL(phys_initrd_start);
unsigned long phys_initrd_size __initdata;
EXPORT_SYMBOL(phys_initrd_size);
unsigned long initrd_start;
EXPORT_SYMBOL(initrd_start);
unsigned long initrd_end;
EXPORT_SYMBOL(initrd_end);

__weak void __init early_init_fdt_scan_reserved_mem(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(early_init_fdt_scan_reserved_mem);

__weak void __module_get(struct module *module)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(__module_get);

__weak int __request_module(bool wait, const char *fmt, ...)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(__request_module);

__weak int __init devtmpfs_init(void)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(devtmpfs_init);

__weak int cap_capable(const struct cred *cred, struct user_namespace *targ_ns,
        int cap, unsigned int opts)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(cap_capable);

__weak int cap_settime(const struct timespec64 *ts, const struct timezone *tz)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(cap_settime);

__weak bool file_ns_capable(const struct file *file, struct user_namespace *ns,
		     int cap)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(file_ns_capable);

__weak bool capable_wrt_inode_uidgid(const struct inode *inode, int cap)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(capable_wrt_inode_uidgid);

__weak bool has_capability_noaudit(struct task_struct *t, int cap)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(has_capability_noaudit);

const kernel_cap_t __cap_empty_set = CAP_EMPTY_SET;
EXPORT_SYMBOL(__cap_empty_set);

int file_caps_enabled = 1;
EXPORT_SYMBOL(file_caps_enabled);

__weak int cap_capget(struct task_struct *target, kernel_cap_t *effective,
           kernel_cap_t *inheritable, kernel_cap_t *permitted)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(cap_capget);

__weak int cap_capset(struct cred *new,
           const struct cred *old,
           const kernel_cap_t *effective,
           const kernel_cap_t *inheritable,
           const kernel_cap_t *permitted)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(cap_capset);

__weak int cap_inode_need_killpriv(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(cap_inode_need_killpriv);

__weak bool mnt_may_suid(struct vfsmount *mnt)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(mnt_may_suid);

bool refcount_dec_and_lock(refcount_t *r, spinlock_t *lock)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(refcount_dec_and_lock);

__weak void free_ipcs(struct ipc_namespace *ns, struct ipc_ids *ids,
	       void (*free)(struct ipc_namespace *, struct kern_ipc_perm *))
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(free_ipcs);

__weak void put_pid_ns(struct pid_namespace *ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(put_pid_ns);

__weak struct pid_namespace *copy_pid_ns(unsigned long flags,
	struct user_namespace *user_ns, struct pid_namespace *old_ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(copy_pid_ns);

__weak void zap_pid_ns_processes(struct pid_namespace *pid_ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(zap_pid_ns_processes);

__weak int mnt_want_write(struct vfsmount *m)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(mnt_want_write);

__weak __latent_entropy
struct mnt_namespace *copy_mnt_ns(unsigned long flags, struct mnt_namespace *ns,
		struct user_namespace *user_ns, struct fs_struct *new_fs)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(copy_mnt_ns);

__weak void mnt_drop_write(struct vfsmount *mnt)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(mnt_drop_write);

__weak int __mnt_want_write(struct vfsmount *m)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__mnt_want_write);

int vfs_getattr(const struct path *path, struct kstat *stat,
        u32 request_mask, unsigned int query_flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vfs_getattr);

__weak int __legitimize_mnt(struct vfsmount *bastard, unsigned seq)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__legitimize_mnt);

__weak struct mount *__lookup_mnt(struct vfsmount *mnt, struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__lookup_mnt);

__weak struct vfsmount *lookup_mnt(const struct path *path)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(lookup_mnt);

__weak int finish_automount(struct vfsmount *m, struct path *path)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(finish_automount);

__weak bool legitimize_mnt(struct vfsmount *bastard, unsigned seq)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(legitimize_mnt);

__weak bool __is_local_mountpoint(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__is_local_mountpoint);

__weak bool current_chrooted(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(current_chrooted);

__weak void __mnt_drop_write_file(struct file *file)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__mnt_drop_write_file);

__weak void kobject_init(struct kobject *kobj, struct kobj_type *ktype)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kobject_init);

__weak void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kmem_cache_alloc);

struct kmem_cache *
kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1] __ro_after_init =
{ /* initialization for https://bugs.llvm.org/show_bug.cgi?id=42570 */ };
EXPORT_SYMBOL(kmalloc_caches);

__weak struct device *get_device(struct device *dev)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(get_device);

__weak void put_device(struct device *dev)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(put_device);

__weak int device_is_dependent(struct device *dev, void *target)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(device_is_dependent);

struct pernet_operations;
__weak int register_pernet_subsys(struct pernet_operations *ops)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(register_pernet_subsys);

__weak kuid_t make_kuid(struct user_namespace *ns, uid_t uid)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(make_kuid);

__weak kgid_t make_kgid(struct user_namespace *ns, gid_t gid)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(make_kgid);

/*
void kfree_skb(struct sk_buff *skb)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kfree_skb);

struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
                int flags, int node)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(__alloc_skb);
*/

__weak int device_create_file(struct device *dev,
		       const struct device_attribute *attr)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(device_create_file);

__weak int device_register(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(device_register);

__weak int subsys_system_register(struct bus_type *subsys,
			   const struct attribute_group **groups)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(subsys_system_register);

__weak void device_unregister(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(device_unregister);

__weak int sysfs_create_bin_file(struct kobject *kobj,
			  const struct bin_attribute *attr)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(sysfs_create_bin_file);

__weak struct kernfs_node *kernfs_find_and_get_ns(struct kernfs_node *parent,
					   const char *name, const void *ns)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(kernfs_find_and_get_ns);

__weak struct kobj_map *kobj_map_init(kobj_probe_t *base_probe, struct mutex *lock)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobj_map_init);

struct kobj_map;
__weak void kobj_unmap(struct kobj_map *domain, dev_t dev, unsigned long range)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobj_unmap);

__weak struct kobject *kobj_lookup(struct kobj_map *domain, dev_t dev, int *index)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobj_lookup);

__weak int device_add(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(device_add);

__weak void device_del(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(device_del);

__weak int __init platform_bus_init(void)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(platform_bus_init);

__weak struct subprocess_info *call_usermodehelper_setup(const char *path, char **argv,
        char **envp, gfp_t gfp_mask,
        int (*init)(struct subprocess_info *info, struct cred *new),
        void (*cleanup)(struct subprocess_info *info),
        void *data)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(call_usermodehelper_setup);

__weak int call_usermodehelper(const char *path, char **argv, char **envp, int wait)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(call_usermodehelper);

__weak void *skb_put(struct sk_buff *skb, unsigned int len)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(skb_put);

__weak void kfree_skb(struct sk_buff *skb)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kfree_skb);

__weak struct sk_buff *__alloc_skb(unsigned int size, gfp_t gfp_mask,
			    int flags, int node)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(__alloc_skb);

__weak void __init files_init(void)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(files_init);

__weak struct sk_buff *skb_copy_expand(const struct sk_buff *skb,
				int newheadroom, int newtailroom,
				gfp_t gfp_mask)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(skb_copy_expand);

__weak void *skb_pull(struct sk_buff *skb, unsigned int len)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(skb_pull);

__weak int netlink_broadcast(struct sock *ssk, struct sk_buff *skb, u32 portid,
              u32 group, gfp_t allocation)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(netlink_broadcast);

__weak int netlink_rcv_skb(struct sk_buff *skb, int (*cb)(struct sk_buff *,
                           struct nlmsghdr *,
                           struct netlink_ext_ack *))
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(netlink_rcv_skb);

__weak void
netlink_kernel_release(struct sock *sk)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(netlink_kernel_release);

__weak bool netlink_ns_capable(const struct sk_buff *skb,
            struct user_namespace *user_ns, int cap)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(netlink_ns_capable);

__weak void __raise_softirq_irqoff(unsigned int nr)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(__raise_softirq_irqoff);

__weak asmlinkage __visible void do_softirq(void)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(do_softirq);

__weak struct sock *
__netlink_kernel_create(struct net *net, int unit, struct module *module,
			struct netlink_kernel_cfg *cfg)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(__netlink_kernel_create);

__weak struct mm_struct *mm_access(struct task_struct *task, unsigned int mode)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(mm_access);

__weak struct net *copy_net_ns(unsigned long flags,
			struct user_namespace *user_ns, struct net *old_net)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(copy_net_ns);

__weak void rhashtable_destroy(struct rhashtable *ht)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(rhashtable_destroy);

__weak int netlink_has_listeners(struct sock *sk, unsigned int group)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(netlink_has_listeners);

__weak void __receive_sock(struct file *file)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(__receive_sock);

void kfree_sensitive(const void *p)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kfree_sensitive);

void kmem_cache_destroy(struct kmem_cache *s)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kmem_cache_destroy);

__weak void *__vmalloc_node(unsigned long size, unsigned long align,
			    gfp_t gfp_mask, int node, const void *caller)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__vmalloc_node);

__weak int is_valid_bugaddr(unsigned long pc)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(is_valid_bugaddr);

bool initcall_debug;
//core_param(initcall_debug, initcall_debug, bool, 0644);
EXPORT_SYMBOL(initcall_debug);

__weak void register_syscore_ops(struct syscore_ops *ops)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(register_syscore_ops);

struct address_space *swapper_spaces[MAX_SWAPFILES] __read_mostly;
EXPORT_SYMBOL(swapper_spaces);

long total_swap_pages;
EXPORT_SYMBOL_GPL(total_swap_pages);

__weak void lru_cache_add(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(lru_cache_add);

__weak unsigned pagevec_lookup_range_tag(struct pagevec *pvec,
		struct address_space *mapping, pgoff_t *index, pgoff_t end,
		xa_mark_t tag)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(pagevec_lookup_range_tag);

__weak void rotate_reclaimable_page(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rotate_reclaimable_page);

__weak void debugfs_remove(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(debugfs_remove);

__weak struct dentry *debugfs_create_file(const char *name, umode_t mode,
				   struct dentry *parent, void *data,
				   const struct file_operations *fops)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(debugfs_create_file);

loff_t default_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(default_llseek);

u32 fsnotify_get_cookie(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(fsnotify_get_cookie);

__weak int expand_stack(struct vm_area_struct *vma, unsigned long address)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(expand_stack);

__weak void __init mmap_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(mmap_init);

__weak void do_trap(struct pt_regs *regs, int signo, int code, unsigned long addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(do_trap);

__weak asmlinkage void do_page_fault(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(do_page_fault);

__weak void vm_stat_account(struct mm_struct *mm, vm_flags_t flags, long npages)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vm_stat_account);

int locks_mandatory_locked(struct file *file)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(locks_mandatory_locked);

__weak struct vm_area_struct *vm_area_alloc(struct mm_struct *mm)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vm_area_alloc);

__weak void vm_area_free(struct vm_area_struct *vma)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vm_area_free);

/* amount of vm to protect from userspace access by both DAC and the LSM*/
unsigned long mmap_min_addr;
EXPORT_SYMBOL(mmap_min_addr);

__weak const struct exception_table_entry *
search_extable(const struct exception_table_entry *base,
	       const size_t num,
	       unsigned long value)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(search_extable);

__weak const struct exception_table_entry *search_module_extables(unsigned long addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(search_module_extables);

__weak void print_modules(void)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(print_modules);

__weak void add_taint(unsigned flag, enum lockdep_ok lockdep_ok)
{
    booter_panic("No impl 'add_taint'.");
}
EXPORT_SYMBOL(add_taint);

__weak void raise_softirq_irqoff(unsigned int nr)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(raise_softirq_irqoff);

__weak struct file *filp_open(const char *filename, int flags, umode_t mode)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(filp_open);

__weak int filp_close(struct file *filp, fl_owner_t id)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(filp_close);

__weak int nonseekable_open(struct inode *inode, struct file *filp)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(nonseekable_open);

__weak int finish_open(struct file *file, struct dentry *dentry,
		int (*open)(struct inode *, struct file *))
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(finish_open);

__weak int vfs_open(const struct path *path, struct file *file)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(vfs_open);

__weak int do_truncate(struct dentry *dentry, loff_t length, unsigned int time_attrs,
	struct file *filp)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(do_truncate);

__weak struct file *do_filp_open(int dfd, struct filename *pathname,
		const struct open_flags *op)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL_GPL(do_filp_open);

__weak char *file_path(struct file *filp, char *buf, int buflen)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(file_path);

__weak struct file *dentry_open(const struct path *path, int flags,
			 const struct cred *cred)
{
    booter_panic("No impl in 'rcu'.");
}
EXPORT_SYMBOL(dentry_open);

__weak void raise_softirq(unsigned int nr)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(raise_softirq);

__weak void __fsnotify_inode_delete(struct inode *inode)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL_GPL(__fsnotify_inode_delete);

__weak int fsnotify(__u32 mask, const void *data, int data_type, struct inode *dir,
	     const struct qstr *file_name, struct inode *inode, u32 cookie)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL_GPL(fsnotify);

__weak int __fsnotify_parent(struct dentry *dentry, __u32 mask, const void *data,
		      int data_type)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL_GPL(__fsnotify_parent);

__weak void fsnotify_sb_delete(struct super_block *sb)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(fsnotify_sb_delete);

__weak int notify_change(struct dentry * dentry, struct iattr * attr, struct inode **delegated_inode)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(notify_change);

__weak int simple_setattr(struct dentry *dentry, struct iattr *iattr)
{
    booter_panic("No impl in 'time'.");
}
EXPORT_SYMBOL(simple_setattr);

__weak int in_group_p(kgid_t grp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(in_group_p);

__weak int groups_search(const struct group_info *group_info, kgid_t grp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(groups_search);

__weak void *krealloc(const void *p, size_t new_size, gfp_t flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(krealloc);

__weak void udelay(unsigned long usecs)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(udelay);

__weak int sysfs_create_group(struct kobject *kobj,
		       const struct attribute_group *grp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(sysfs_create_group);

__weak void sysfs_remove_group(struct kobject *kobj,
			const struct attribute_group *grp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(sysfs_remove_group);

struct clk;
unsigned long clk_get_rate(struct clk *clk)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(clk_get_rate);

__weak int of_clk_set_defaults(struct device_node *node, bool clk_supplier)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(of_clk_set_defaults);

__weak int of_irq_to_resource_table(struct device_node *dev, struct resource *res,
		int nr_irqs)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(of_irq_to_resource_table);

__weak int of_irq_get_byname(struct device_node *dev, const char *name)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(of_irq_get_byname);

__weak int of_irq_get(struct device_node *dev, int index)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(of_irq_get);

__weak void of_msi_configure(struct device *dev, struct device_node *np)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(of_msi_configure);

__weak int of_irq_count(struct device_node *dev)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(of_irq_count);

int __anon_vma_prepare(struct vm_area_struct *vma)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__anon_vma_prepare);

void unlink_anon_vmas(struct vm_area_struct *vma)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unlink_anon_vmas);

void pmd_clear_bad(pmd_t *pmd)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(pmd_clear_bad);

int swap_readpage(struct page *page, bool synchronous)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(swap_readpage);

void tlb_finish_mmu(struct mmu_gather *tlb,
        unsigned long start, unsigned long end)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(tlb_finish_mmu);

void workingset_refault(struct page *page, void *shadow)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(workingset_refault);

long get_user_pages_remote(struct mm_struct *mm,
        unsigned long start, unsigned long nr_pages,
        unsigned int gup_flags, struct page **pages,
        struct vm_area_struct **vmas, int *locked)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(get_user_pages_remote);

void tlb_gather_mmu(struct mmu_gather *tlb, struct mm_struct *mm,
            unsigned long start, unsigned long end)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(tlb_gather_mmu);

int __page_mapcount(struct page *page)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__page_mapcount);

void page_add_file_rmap(struct page *page, bool compound)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(page_add_file_rmap);

int migrate_page(struct address_space *mapping,
        struct page *newpage, struct page *page,
        enum migrate_mode mode)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(migrate_page);

int swap_writepage(struct page *page, struct writeback_control *wbc)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(swap_writepage);

swp_entry_t get_swap_page(struct page *page)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(get_swap_page);






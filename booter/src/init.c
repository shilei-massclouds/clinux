// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/cache.h>
#include <linux/jiffies.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/ipc_namespace.h>
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

const struct sysfs_ops kobj_sysfs_ops;
EXPORT_SYMBOL(kobj_sysfs_ops);

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

void __put_page(struct page *page)
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

loff_t noop_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(noop_llseek);

int setattr_prepare(struct dentry *dentry, struct iattr *attr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(setattr_prepare);

void generic_fillattr(struct inode *inode, struct kstat *stat)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_fillattr);

void setattr_copy(struct inode *inode, const struct iattr *attr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(setattr_copy);

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

size_t copy_page_to_iter(struct page *page, size_t offset, size_t bytes,
             struct iov_iter *i)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(copy_page_to_iter);

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

void *kvmalloc_node(size_t size, gfp_t flags, int node)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kvmalloc_node);

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

void __mnt_drop_write(struct vfsmount *mnt)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(__mnt_drop_write);

int filp_close(struct file *filp, fl_owner_t id)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(filp_close);

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

void lru_add_drain(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(lru_add_drain);

__weak bool path_noexec(const struct path *path)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(path_noexec);

int __fsnotify_parent(struct dentry *dentry, __u32 mask, const void *data,
              int data_type)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL_GPL(__fsnotify_parent);

struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL_GPL(find_vma);

struct file *file_open_root(struct dentry *dentry, struct vfsmount *mnt,
                const char *filename, int flags, umode_t mode)
{
    booter_panic("No impl in 'workqueue'.");
}
EXPORT_SYMBOL(file_open_root);

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

int notify_change(struct dentry * dentry, struct iattr * attr, struct inode **delegated_inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(notify_change);

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

bool mnt_may_suid(struct vfsmount *mnt)
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

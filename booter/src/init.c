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
#include <linux/xattr.h>
#include <linux/fs_context.h>
#include <linux/ipc_namespace.h>
#include <linux/user_namespace.h>
#include <linux/rmap.h>
#include <linux/proc_fs.h>
#include <linux/kobj_map.h>
#include <linux/backing-dev-defs.h>
#include <linux/backing-dev.h>
#include <linux/parser.h>
#include <linux/poll.h>
#include <net/netlink.h>
#include <net/icmp.h>
#include <net/icmp.h>
#include <net/protocol.h>
#include <net/inet_hashtables.h>
#include <net/dst_metadata.h>
#include <asm/sbi.h>
#include <asm/current.h>
#ifdef GENERIC_TIME_VSYSCALL
#include <../vdso/datapage.h>
#else
#include <asm/vdso.h>
#endif
#include <cl_hook.h>
#include "booter.h"
#include "../fs/internal.h"
#include "../ipc/util.h"

#define UL_STR_SIZE 19  /* prefix with '0x' and end with '\0' */

__visible u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

extern int cl_top_init();

void
start_kernel(void)
{
    cl_top_init();
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

/*
int
task_work_add(struct task_struct *task, struct callback_head *work, int notify)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(task_work_add);
*/

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

__weak int setattr_prepare(struct dentry *dentry, struct iattr *attr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(setattr_prepare);

/*
void generic_fillattr(struct inode *inode, struct kstat *stat)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(generic_fillattr);
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

/*
errseq_t errseq_set(errseq_t *eseq, int err)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(errseq_set);
*/

__weak int send_sig(int sig, struct task_struct *p, int priv)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(send_sig);

__weak void wakeup_flusher_threads(enum wb_reason reason)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wakeup_flusher_threads);

__weak void blk_finish_plug(struct blk_plug *plug)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_finish_plug);

__weak void blk_start_plug(struct blk_plug *plug)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_start_plug);

/*
 * Flag that makes the machine dump writes/reads and block dirtyings.
 */
int block_dump;
EXPORT_SYMBOL(block_dump);

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
void free_uid(struct user_struct *up)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(free_uid);
*/

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

__weak char *d_path(const struct path *path, char *buf, int buflen)
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

__weak loff_t no_llseek(struct file *file, loff_t offset, int whence)
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

/*
bool cancel_work_sync(struct work_struct *work)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cancel_work_sync);
*/

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

/*
void cgroup_enter_frozen(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cgroup_enter_frozen);
*/

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

/*
void cgroup_leave_frozen(bool always_leave)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(cgroup_leave_frozen);
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

/*
void exit_shm(struct task_struct *task)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(exit_shm);
*/

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

/*
void exit_itimers(struct signal_struct *sig)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(exit_itimers);
*/

/*
void vfree(const void *addr)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(vfree);
*/

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

/*
__weak void __init early_init_fdt_scan_reserved_mem(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(early_init_fdt_scan_reserved_mem);
*/

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

/*
int vfs_getattr(const struct path *path, struct kstat *stat,
        u32 request_mask, unsigned int query_flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vfs_getattr);
*/

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

//struct kmem_cache *
//kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1] __ro_after_init =
//{ /* initialization for https://bugs.llvm.org/show_bug.cgi?id=42570 */ };
//EXPORT_SYMBOL(kmalloc_caches);

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

/*
void kmem_cache_destroy(struct kmem_cache *s)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kmem_cache_destroy);
*/

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

__weak struct dentry *debugfs_create_dir(const char *name, struct dentry *parent)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(debugfs_create_dir);

__weak struct dentry *debugfs_create_file(const char *name, umode_t mode,
				   struct dentry *parent, void *data,
				   const struct file_operations *fops)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(debugfs_create_file);

/*
u32 fsnotify_get_cookie(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(fsnotify_get_cookie);
*/

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

__weak int locks_mandatory_locked(struct file *file)
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

/*
struct clk;
unsigned long clk_get_rate(struct clk *clk)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(clk_get_rate);
*/

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

__weak int __anon_vma_prepare(struct vm_area_struct *vma)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__anon_vma_prepare);

__weak void unlink_anon_vmas(struct vm_area_struct *vma)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(unlink_anon_vmas);

int swap_readpage(struct page *page, bool synchronous)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(swap_readpage);

__weak void workingset_refault(struct page *page, void *shadow)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(workingset_refault);

int __page_mapcount(struct page *page)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(__page_mapcount);

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

__weak int sock_register(const struct net_proto_family *ops)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(sock_register);

ssize_t generic_file_splice_read(struct file *in, loff_t *ppos,
                 struct pipe_inode_info *pipe, size_t len,
                 unsigned int flags)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(generic_file_splice_read);

__weak void __init files_maxfiles_init(void)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(files_maxfiles_init);

/*
void blk_flush_plug_list(struct blk_plug *plug, bool from_schedule)
{
    booter_panic("No impl 'sched'.");
}
EXPORT_SYMBOL(blk_flush_plug_list);
*/

__weak int write_inode_now(struct inode *inode, int sync)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(write_inode_now);

__weak void inode_wait_for_writeback(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inode_wait_for_writeback);

__weak void sb_mark_inode_writeback(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sb_mark_inode_writeback);

__weak void sb_clear_inode_writeback(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sb_clear_inode_writeback);

__weak void wakeup_flusher_threads_bdi(struct backing_dev_info *bdi,
				enum wb_reason reason)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wakeup_flusher_threads_bdi);

__weak void wb_start_background_writeback(struct bdi_writeback *wb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wb_start_background_writeback);

__weak int __break_lease(struct inode *inode, unsigned int mode, unsigned int type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__break_lease);

__weak void
locks_free_lock_context(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(locks_free_lock_context);

__weak int locks_mandatory_area(struct inode *inode, struct file *filp, loff_t start,
			 loff_t end, unsigned char type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(locks_mandatory_area);

__weak void truncate_inode_pages_final(struct address_space *mapping)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(truncate_inode_pages_final);

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

__weak void locks_remove_posix(struct file *filp, fl_owner_t owner)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(locks_remove_posix);

__weak void locks_remove_file(struct file *filp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(locks_remove_file);

/*
void __f_setown(struct file *filp, struct pid *pid, enum pid_type type,
        int force)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__f_setown);
*/

__weak void __do_page_cache_readahead(struct address_space *mapping,
		struct file *file, pgoff_t index, unsigned long nr_to_read,
		unsigned long lookahead_size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(__do_page_cache_readahead);

__weak void page_cache_sync_readahead(struct address_space *mapping,
			       struct file_ra_state *ra, struct file *filp,
			       pgoff_t index, unsigned long req_count)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(page_cache_sync_readahead);

__weak void
page_cache_async_readahead(struct address_space *mapping,
			   struct file_ra_state *ra, struct file *filp,
			   struct page *page, pgoff_t index,
			   unsigned long req_count)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(page_cache_async_readahead);


__weak void blkdev_put(struct block_device *bdev, fmode_t mode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blkdev_put);

struct super_block *blockdev_superblock __read_mostly;
EXPORT_SYMBOL_GPL(blockdev_superblock);

__weak struct block_device *I_BDEV(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(I_BDEV);

__weak void sysfs_remove_files(struct kobject *kobj, const struct attribute * const *ptr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(sysfs_remove_files);

__weak int sysfs_create_files(struct kobject *kobj, const struct attribute * const *ptr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(sysfs_create_files);

__weak int bdev_read_only(struct block_device *bdev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdev_read_only);

__weak void bdput(struct block_device *bdev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdput);

__weak int sync_blockdev(struct block_device *bdev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sync_blockdev);

struct backing_dev_info noop_backing_dev_info = {
	.capabilities	= BDI_CAP_NO_ACCT_AND_WRITEBACK,
};
EXPORT_SYMBOL_GPL(noop_backing_dev_info);

__weak void wb_workfn(struct work_struct *work)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wb_workfn);

__weak void laptop_mode_timer_fn(struct timer_list *t)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(laptop_mode_timer_fn);

__weak struct hd_struct *disk_map_sector_rcu(struct gendisk *disk, sector_t sector)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(disk_map_sector_rcu);

__weak struct hd_struct *__disk_get_part(struct gendisk *disk, int partno)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__disk_get_part);

__weak int bdi_set_min_ratio(struct backing_dev_info *bdi, unsigned int min_ratio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdi_set_min_ratio);

__weak int bdi_set_max_ratio(struct backing_dev_info *bdi, unsigned max_ratio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdi_set_max_ratio);

__weak int revalidate_disk(struct gendisk *disk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(revalidate_disk);

__weak void shrink_dcache_sb(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shrink_dcache_sb);

__weak void invalidate_bdev(struct block_device *bdev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(invalidate_bdev);

__weak int __invalidate_device(struct block_device *bdev, bool kill_dirty)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__invalidate_device);

__weak int hd_ref_init(struct hd_struct *part)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(hd_ref_init);

__weak struct block_device *bdget(dev_t dev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdget);

__weak void delete_partition(struct gendisk *disk, struct hd_struct *part)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(delete_partition);

/*
 * Flag that puts the machine in "laptop mode". Doubles as a timeout in jiffies:
 * a full sync is triggered after this time elapses without any disk activity.
 */
int laptop_mode;
EXPORT_SYMBOL(laptop_mode);

/*
void put_io_context(struct io_context *ioc)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_io_context);
*/

/*
void elevator_init_mq(struct request_queue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elevator_init_mq);
*/

__weak blk_qc_t submit_bio(struct bio *bio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(submit_bio);

__weak blk_qc_t submit_bio_noacct(struct bio *bio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(submit_bio_noacct);

__weak int blk_status_to_errno(blk_status_t status)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(blk_status_to_errno);

__weak char *disk_name(struct gendisk *hd, int partno, char *buf)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(disk_name);

unsigned long __read_mostly sysctl_hung_task_timeout_secs = CONFIG_DEFAULT_HUNG_TASK_TIMEOUT;
EXPORT_SYMBOL(sysctl_hung_task_timeout_secs);

__weak const char *bdevname(struct block_device *bdev, char *buf)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdevname);

__weak unsigned long invalidate_mapping_pages(struct address_space *mapping,
		pgoff_t start, pgoff_t end)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(invalidate_mapping_pages);

__weak int page_mkclean(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(page_mkclean);

__weak bool try_to_unmap(struct page *page, enum ttu_flags flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(try_to_unmap);

/*
pte_t ptep_clear_flush(struct vm_area_struct *vma, unsigned long address,
               pte_t *ptep)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ptep_clear_flush);
*/

__cacheline_aligned_in_smp DEFINE_SPINLOCK(mmlist_lock);
EXPORT_SYMBOL(mmlist_lock);

__weak void dump_vma(const struct vm_area_struct *vma)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(dump_vma);

__weak void dump_page(struct page *page, const char *reason)
{
    booter_panic("No impl 'dump_page'.");
}
EXPORT_SYMBOL(dump_page);

__weak int anon_vma_clone(struct vm_area_struct *dst, struct vm_area_struct *src)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(anon_vma_clone);

__weak void workingset_update_node(struct xa_node *node)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(workingset_update_node);

__weak int invalidate_inode_pages2_range(struct address_space *mapping,
				  pgoff_t start, pgoff_t end)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(invalidate_inode_pages2_range);

__weak void bd_forget(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bd_forget);

__weak struct mm_struct *get_task_mm(struct task_struct *task)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(get_task_mm);

__weak void mmput(struct mm_struct *mm)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(mmput);

__weak void elevator_init_mq(struct request_queue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elevator_init_mq);

__weak bool blk_rq_merge_ok(struct request *rq, struct bio *bio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_rq_merge_ok);

__weak enum elv_merge blk_try_merge(struct request *rq, struct bio *bio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blk_try_merge);

__weak int elevator_switch_mq(struct request_queue *q,
			      struct elevator_type *new_e)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(elevator_switch_mq);

/*
void blk_mq_sched_free_requests(struct request_queue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(blk_mq_sched_free_requests);
*/

__weak void bdi_put(struct backing_dev_info *bdi)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdi_put);

__weak struct backing_dev_info *bdi_alloc(int node_id)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdi_alloc);

__weak unsigned long wb_calc_thresh(struct bdi_writeback *wb, unsigned long thresh)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wb_calc_thresh);

__weak struct block_device *bdget_disk(struct gendisk *disk, int partno)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bdget_disk);

__weak int blkdev_get(struct block_device *bdev, fmode_t mode, void *holder)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(blkdev_get);

__weak int sb_set_blocksize(struct super_block *sb, int size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sb_set_blocksize);

int vfs_fsync_range(struct file *file, loff_t start, loff_t end, int datasync)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vfs_fsync_range);

__weak int fsync_bdev(struct block_device *bdev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fsync_bdev);

__weak int inode_has_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inode_has_buffers);

__weak int try_to_free_buffers(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(try_to_free_buffers);

__weak int thaw_bdev(struct block_device *bdev, struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(thaw_bdev);

__weak pgoff_t __page_file_index(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(__page_file_index);

__weak int __set_page_dirty_buffers(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__set_page_dirty_buffers);

int buffer_heads_over_limit;
EXPORT_SYMBOL(buffer_heads_over_limit);

__weak int remove_inode_buffers(struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(remove_inode_buffers);

__weak void emergency_thaw_bdev(struct super_block *sb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(emergency_thaw_bdev);

__weak void pagecache_isize_extended(struct inode *inode, loff_t from, loff_t to)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(pagecache_isize_extended);

__weak void do_invalidatepage(struct page *page, unsigned int offset,
		       unsigned int length)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(do_invalidatepage);

__weak struct request *elv_latter_request(struct request_queue *q, struct request *rq)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_latter_request);

__weak struct request *elv_former_request(struct request_queue *q, struct request *rq)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_former_request);

__weak void elv_merge_requests(struct request_queue *q, struct request *rq,
			     struct request *next)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_merge_requests);

__weak void blk_mq_unquiesce_queue(struct request_queue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(blk_mq_unquiesce_queue);

__weak void blk_mq_quiesce_queue_nowait(struct request_queue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(blk_mq_quiesce_queue_nowait);

__weak enum elv_merge elv_merge(struct request_queue *q, struct request **req,
		struct bio *bio)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_merge);

__weak bool elv_attempt_insert_merge(struct request_queue *q, struct request *rq)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_attempt_insert_merge);

__weak void elv_merged_request(struct request_queue *q, struct request *rq,
		enum elv_merge type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(elv_merged_request);

__weak void workingset_activation(struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(workingset_activation);

__weak void kvfree(const void *addr)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(kvfree);

__weak bool is_vmalloc_addr(const void *x)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(is_vmalloc_addr);

__weak void vfree(const void *addr)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(vfree);

__weak void blk_mq_quiesce_queue(struct request_queue *q)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL_GPL(blk_mq_quiesce_queue);

__weak ssize_t elv_iosched_store(struct request_queue *q, const char *name,
			  size_t count)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(elv_iosched_store);

__weak void __elevator_exit(struct request_queue *q, struct elevator_queue *e)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(__elevator_exit);

__weak ssize_t elv_iosched_show(struct request_queue *q, char *name)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(elv_iosched_show);

__weak void elv_unregister_queue(struct request_queue *q)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(elv_unregister_queue);

__weak int elv_register_queue(struct request_queue *q, bool uevent)
{
    booter_panic("No impl 'percpu'.");
}
EXPORT_SYMBOL(elv_register_queue);

ssize_t
iter_file_splice_write(struct pipe_inode_info *pipe, struct file *out,
              loff_t *ppos, size_t len, unsigned int flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(iter_file_splice_write);

__weak int __mnt_want_write_file(struct file *file)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(__mnt_want_write_file);

int match_int(substring_t *s, int *result)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(match_int);

__weak void mpage_readahead(struct readahead_control *rac, get_block_t get_block)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(mpage_readahead);

/*
bool is_bad_inode(struct inode *inode)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(is_bad_inode);
*/

int match_token(char *s, const match_table_t table, substring_t args[])
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(match_token);

__weak void truncate_inode_pages(struct address_space *mapping, loff_t lstart)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(truncate_inode_pages);

__weak int filemap_flush(struct address_space *mapping)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(filemap_flush);

__weak struct block_device *blkdev_get_by_path(const char *path, fmode_t mode,
					void *holder)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(blkdev_get_by_path);

__weak int mpage_writepage(struct page *page, get_block_t get_block,
	struct writeback_control *wbc)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(mpage_writepage);

__weak void clean_page_buffers(struct page *page)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(clean_page_buffers);

__weak int bdi_register_va(struct backing_dev_info *bdi, const char *fmt, va_list args)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(bdi_register_va);

// From backing_dev.
/*
 * bdi_lock protects bdi_tree and updates to bdi_list. bdi_list has RCU
 * reader side locking.
 */
DEFINE_SPINLOCK(bdi_lock);
EXPORT_SYMBOL(bdi_lock);
LIST_HEAD(bdi_list);
EXPORT_SYMBOL(bdi_list);

__weak struct address_space *page_mapping(struct page *page)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(page_mapping);

__weak int get_user_pages_fast(unsigned long start, int nr_pages,
			unsigned int gup_flags, struct page **pages)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(get_user_pages_fast);

void mlock_vma_page(struct page *page)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(mlock_vma_page);

__weak long populate_vma_page_range(struct vm_area_struct *vma,
		unsigned long start, unsigned long end, int *locked)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(populate_vma_page_range);

__weak int __mm_populate(unsigned long start, unsigned long len, int ignore_errors)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__mm_populate);

void migration_entry_wait(struct mm_struct *mm, pmd_t *pmd,
                unsigned long address)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(migration_entry_wait);

__weak struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(find_vma);

__weak loff_t noop_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(noop_llseek);

__weak loff_t
generic_file_llseek_size(struct file *file, loff_t offset, int whence,
		loff_t maxsize, loff_t eof)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(generic_file_llseek_size);

__weak loff_t generic_file_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(generic_file_llseek);

__weak int shmem_init_fs_context(struct fs_context *fc)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(shmem_init_fs_context);

__weak ssize_t rw_copy_check_uvector(int type, const struct iovec __user * uvector,
			      unsigned long nr_segs, unsigned long fast_segs,
			      struct iovec *fast_pointer,
			      struct iovec **ret_pointer)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(rw_copy_check_uvector);

__weak int simple_xattr_set(struct simple_xattrs *xattrs, const char *name,
             const void *value, size_t size, int flags,
             ssize_t *removed_size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_set);

__weak ssize_t
__vfs_getxattr(struct dentry *dentry, struct inode *inode, const char *name,
	       void *value, size_t size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__vfs_getxattr);

__weak ssize_t
vfs_getxattr_alloc(struct dentry *dentry, const char *name, char **xattr_value,
		   size_t xattr_size, gfp_t flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vfs_getxattr_alloc);

__weak int
__vfs_removexattr(struct dentry *dentry, const char *name)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__vfs_removexattr);

__weak const char *xattr_full_name(const struct xattr_handler *handler,
                const char *name)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(xattr_full_name);

__weak int simple_xattr_get(struct simple_xattrs *xattrs, const char *name,
             void *buffer, size_t size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_get);

__weak ssize_t simple_xattr_list(struct inode *inode, struct simple_xattrs *xattrs,
              char *buffer, size_t size)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_xattr_list);

void plist_add(struct plist_node *node, struct plist_head *head)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(plist_add);

__weak void sigqueue_free(struct sigqueue *q)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sigqueue_free);

__weak struct sigqueue *sigqueue_alloc(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sigqueue_alloc);

__weak int send_sigqueue(struct sigqueue *q, struct pid *pid, enum pid_type type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(send_sigqueue);

#if defined(HAVE_ARCH_PICK_MMAP_LAYOUT) || \
    defined(CONFIG_ARCH_WANT_DEFAULT_TOPDOWN_MMAP_LAYOUT)
int sysctl_legacy_va_layout;
EXPORT_SYMBOL(sysctl_legacy_va_layout);
#endif

/* enforced gap between the expanding stack and other mappings. */
unsigned long stack_guard_gap = 256UL<<PAGE_SHIFT;
EXPORT_SYMBOL(stack_guard_gap);

__weak unsigned long
arch_get_unmapped_area_topdown(struct file *filp, unsigned long addr,
			  unsigned long len, unsigned long pgoff,
			  unsigned long flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(arch_get_unmapped_area_topdown);

#ifdef CONFIG_HAVE_ARCH_MMAP_RND_BITS
const int mmap_rnd_bits_min = CONFIG_ARCH_MMAP_RND_BITS_MIN;
const int mmap_rnd_bits_max = CONFIG_ARCH_MMAP_RND_BITS_MAX;
int mmap_rnd_bits __read_mostly = CONFIG_ARCH_MMAP_RND_BITS;
EXPORT_SYMBOL(mmap_rnd_bits);
#endif

__weak unsigned long
arch_get_unmapped_area(struct file *filp, unsigned long addr,
		unsigned long len, unsigned long pgoff, unsigned long flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(arch_get_unmapped_area);

__weak unsigned long
randomize_page(unsigned long start, unsigned long range)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(randomize_page);

/* /sys/fs */
struct kobject *fs_kobj;
EXPORT_SYMBOL_GPL(fs_kobj);

__weak struct vm_area_struct *vm_area_dup(struct vm_area_struct *orig)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(vm_area_dup);

__weak void free_pages_and_swap_cache(struct page **pages, int nr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(free_pages_and_swap_cache);

__weak unsigned long do_mmap(struct file *file, unsigned long addr,
			unsigned long len, unsigned long prot,
			unsigned long flags, unsigned long pgoff,
			unsigned long *populate, struct list_head *uf)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(do_mmap);

__weak void page_add_file_rmap(struct page *page, bool compound)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(page_add_file_rmap);

__weak unsigned long shmem_get_unmapped_area(struct file *file,
				      unsigned long uaddr, unsigned long len,
				      unsigned long pgoff, unsigned long flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shmem_get_unmapped_area);

__weak int shmem_zero_setup(struct vm_area_struct *vma)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shmem_zero_setup);

/*
 * The vDSO data page.
 */
static union {
	struct vdso_data	data;
	u8			page[PAGE_SIZE];
} vdso_data_store __page_aligned_data;
struct vdso_data *vdso_data = &vdso_data_store.data;
EXPORT_SYMBOL(vdso_data);

// From kernel/sys
/*
 * the same as above, but for filesystems which can only store a 16-bit
 * UID and GID. as such, this is needed on all architectures
 */

int fs_overflowuid = DEFAULT_FS_OVERFLOWUID;
int fs_overflowgid = DEFAULT_FS_OVERFLOWGID;

EXPORT_SYMBOL(fs_overflowuid);
EXPORT_SYMBOL(fs_overflowgid);

// From kernel/sys
/*
 * this is where the system-wide overflow UID and GID are defined, for
 * architectures that now have 32-bit UID/GID but didn't in the past
 */

int overflowuid = DEFAULT_OVERFLOWUID;
int overflowgid = DEFAULT_OVERFLOWGID;
EXPORT_SYMBOL(overflowuid);
EXPORT_SYMBOL(overflowgid);

// From kernel/sys
DECLARE_RWSEM(uts_sem);
EXPORT_SYMBOL(uts_sem);

__weak void signal_wake_up_state(struct task_struct *t, unsigned int state)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(signal_wake_up_state);

int show_unhandled_signals = 1;
EXPORT_SYMBOL(show_unhandled_signals);

__weak void rtnl_unlock(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_unlock);

__weak int rtnl_is_locked(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_is_locked);

__weak int rtnl_trylock(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_trylock);

__weak int rtnl_lock_killable(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_lock_killable);

__weak void rtnl_lock(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_lock);

__weak void net_ns_get_ownership(const struct net *net, kuid_t *uid, kgid_t *gid)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(net_ns_get_ownership);

__weak int peernet2id_alloc(struct net *net, struct net *peer, gfp_t gfp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(peernet2id_alloc);

__weak int register_pernet_device(struct pernet_operations *ops)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(register_pernet_device);

__weak int peernet2id(const struct net *net, struct net *peer)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(peernet2id);

__weak struct net *get_net_ns_by_pid(pid_t pid)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(get_net_ns_by_pid);

__weak struct net *get_net_ns_by_fd(int fd)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(get_net_ns_by_fd);

/*
 * pernet_ops_rwsem: protects: pernet_list, net_generic_ids,
 * init_net_initialized and first_device pointer.
 * This is internal net namespace object. Please, don't use it
 * outside.
 */
DECLARE_RWSEM(pernet_ops_rwsem);
EXPORT_SYMBOL_GPL(pernet_ops_rwsem);

__weak bool peernet_has_id(const struct net *net, struct net *peer)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(peernet_has_id);

__weak void net_drop_ns(void *p)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(net_drop_ns);

__weak struct net *get_net_ns_by_id(const struct net *net, int id)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(get_net_ns_by_id);

__weak void rtmsg_ifinfo(int type, struct net_device *dev, unsigned int change,
		  gfp_t flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtmsg_ifinfo);

__weak struct sk_buff *rtmsg_ifinfo_build_skb(int type, struct net_device *dev,
				       unsigned int change,
				       u32 event, gfp_t flags, int *new_nsid,
				       int new_ifindex)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtmsg_ifinfo_build_skb);

__weak int skb_copy_datagram_iter(const struct sk_buff *skb, int offset,
               struct iov_iter *to, int len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(skb_copy_datagram_iter);

__weak int sk_filter_trim_cap(struct sock *sk, struct sk_buff *skb, unsigned int cap)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sk_filter_trim_cap);

__weak void rtmsg_ifinfo_newnet(int type, struct net_device *dev, unsigned int change,
			 gfp_t flags, int *new_nsid, int new_ifindex)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtmsg_ifinfo_newnet);

__weak void __rtnl_unlock(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__rtnl_unlock);

int put_cmsg(struct msghdr * msg, int level, int type, int len, void *data)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_cmsg);

int net_ratelimit(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(net_ratelimit);

__weak void rtmsg_ifinfo_send(struct sk_buff *skb, struct net_device *dev, gfp_t flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtmsg_ifinfo_send);

__weak int set_user_sigmask(const sigset_t __user *umask, size_t sigsetsize)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(set_user_sigmask);

__weak void __set_current_blocked(const sigset_t *newset)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__set_current_blocked);

__weak int do_send_sig_info(int sig, struct kernel_siginfo *info, struct task_struct *p,
			enum pid_type type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(do_send_sig_info);

__weak bool __oom_reap_task_mm(struct mm_struct *mm)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__oom_reap_task_mm);

__weak void shm_init_ns(struct ipc_namespace *ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shm_init_ns);

__weak void shm_exit_ns(struct ipc_namespace *ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shm_exit_ns);

int ipc_mni = IPCMNI;
EXPORT_SYMBOL(ipc_mni);

__weak void __init sem_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sem_init);

__weak void __init shm_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(shm_init);

__weak void __mmdrop(struct mm_struct *mm)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(__mmdrop);

__weak int nr_processes(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(nr_processes);

__weak struct ipc_namespace *copy_ipcs(unsigned long flags,
	struct user_namespace *user_ns, struct ipc_namespace *ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(copy_ipcs);

__weak void put_ipc_ns(struct ipc_namespace *ns)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_ipc_ns);

__weak int copy_semundo(unsigned long clone_flags, struct task_struct *tsk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(copy_semundo);

__weak void exit_sem(struct task_struct *tsk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(exit_sem);

__weak struct proc_dir_entry *proc_create_data(const char *name, umode_t mode,
		struct proc_dir_entry *parent,
		const struct proc_ops *proc_ops, void *data)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_create_data);

__weak struct proc_dir_entry *proc_mkdir(const char *name,
		struct proc_dir_entry *parent)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_mkdir);

__weak void *PDE_DATA(const struct inode *inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(PDE_DATA);

__weak int dequeue_signal(struct task_struct *tsk, sigset_t *mask, kernel_siginfo_t *info)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(dequeue_signal);

__weak int next_signal(struct sigpending *pending, sigset_t *mask)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(next_signal);

__weak enum siginfo_layout siginfo_layout(unsigned sig, int si_code)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(siginfo_layout);

int anon_inode_getfd(const char *name, const struct file_operations *fops,
             void *priv, int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(anon_inode_getfd);

__weak int fcntl_getlease(struct file *filp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fcntl_getlease);

__weak int fcntl_setlease(unsigned int fd, struct file *filp, long arg)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fcntl_setlease);

__weak int fcntl_setlk(unsigned int fd, struct file *filp, unsigned int cmd,
		struct flock *flock)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fcntl_setlk);

__weak int fcntl_getlk(struct file *filp, unsigned int cmd, struct flock *flock)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fcntl_getlk);

__weak int fcntl_dirnotify(int fd, struct file *filp, unsigned long arg)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fcntl_dirnotify);

/*
__poll_t datagram_poll(struct file *file, struct socket *sock,
               poll_table *wait)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(datagram_poll);
*/

__weak void dst_release(struct dst_entry *dst)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(dst_release);

__weak int tcp_v4_early_demux(struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_v4_early_demux);

__weak struct rtable *ip_route_output_flow(struct net *net, struct flowi4 *flp4,
                    const struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ip_route_output_flow);

__weak int tcp_v4_err(struct sk_buff *skb, u32 info)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_v4_err);

__weak int __init tcp4_proc_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp4_proc_init);

__weak void tcp4_proc_exit(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp4_proc_exit);

__weak int tcp_v4_rcv(struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_v4_rcv);

__weak int inet_ctl_sock_create(struct sock **sk, unsigned short family,
			 unsigned short type, unsigned char protocol,
			 struct net *net)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_ctl_sock_create);

__weak int inet_sk_rebuild_header(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_sk_rebuild_header);

__weak struct proc_dir_entry *proc_create(const char *name, umode_t mode,
				   struct proc_dir_entry *parent,
				   const struct proc_ops *proc_ops)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_create);

__weak int ip_getsockopt(struct sock *sk, int level,
          int optname, char __user *optval, int __user *optlen)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_getsockopt);

__weak void ip_icmp_error(struct sock *sk, struct sk_buff *skb, int err,
           __be16 port, u32 info, u8 *payload)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_icmp_error);

__weak void inet_sock_destruct(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_sock_destruct);

__weak int ip_recv_error(struct sock *sk, struct msghdr *msg, int len, int *addr_len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_recv_error);

__weak int ip_setsockopt(struct sock *sk, int level, int optname, sockptr_t optval,
        unsigned int optlen)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_setsockopt);

void reuseport_detach_sock(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(reuseport_detach_sock);

DEFINE_STATIC_KEY_FALSE(bpf_stats_enabled_key);
EXPORT_SYMBOL(bpf_stats_enabled_key);

const struct ipv6_stub *ipv6_stub __read_mostly;
EXPORT_SYMBOL(ipv6_stub);

__weak void ipv4_sk_redirect(struct sk_buff *skb, struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ipv4_sk_redirect);

__weak struct rtable *ip_route_output_key_hash(struct net *net, struct flowi4 *fl4,
					const struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ip_route_output_key_hash);

bool __do_once_start(bool *done, unsigned long *flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(__do_once_start);

/*
struct sk_buff *skb_recv_datagram(struct sock *sk, unsigned int flags,
                  int noblock, int *err)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(skb_recv_datagram);
*/

__weak void ipv4_sk_update_pmtu(struct sk_buff *skb, struct sock *sk, u32 mtu)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ipv4_sk_update_pmtu);

__weak void __icmp_send(struct sk_buff *skb_in, int type, int code, __be32 info,
         const struct ip_options *opt)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(__icmp_send);

/*
int ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip4_datagram_connect);
*/

/*
void ip_flush_pending_frames(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_flush_pending_frames);
*/

__weak int ip_mc_sf_allow(struct sock *sk, __be32 loc_addr, __be32 rmt_addr,
           int dif, int sdif)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_mc_sf_allow);

__weak void ip_cmsg_recv_offset(struct msghdr *msg, struct sock *sk,
             struct sk_buff *skb, int tlen, int offset)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_cmsg_recv_offset);

__weak int ip_check_mc_rcu(struct in_device *in_dev, __be32 mc_addr, __be32 src_addr, u8 proto)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_check_mc_rcu);

__weak int ip_mc_validate_source(struct sk_buff *skb, __be32 daddr, __be32 saddr,
			  u8 tos, struct net_device *dev,
			  struct in_device *in_dev, u32 *itag)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_mc_validate_source);

__weak void ipv4_pktinfo_prepare(const struct sock *sk, struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ipv4_pktinfo_prepare);

int proc_dointvec_ms_jiffies(struct ctl_table *table, int write, void *buffer,
        size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_dointvec_ms_jiffies);

__weak int ip_cmsg_send(struct sock *sk, struct msghdr *msg, struct ipcm_cookie *ipc,
         bool allow_ipv6)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_cmsg_send);

/*
void ip4_datagram_release_cb(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip4_datagram_release_cb);
*/

int proc_dointvec_jiffies(struct ctl_table *table, int write,
              void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_dointvec_jiffies);

/*
void skb_free_datagram(struct sock *sk, struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(skb_free_datagram);
*/

void __do_once_done(bool *done, struct static_key_true *once_key,
            unsigned long *flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__do_once_done);

__weak int inet_recv_error(struct sock *sk, struct msghdr *msg, int len, int *addr_len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_recv_error);

__weak void icmp_out_count(struct net *net, unsigned char type)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(icmp_out_count);

struct net_protocol __rcu *inet_protos[MAX_INET_PROTOS] __read_mostly;
EXPORT_SYMBOL(inet_protos);

__weak void __init arp_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(arp_init);

__weak int ip_route_input_noref(struct sk_buff *skb, __be32 daddr, __be32 saddr,
			 u8 tos, struct net_device *dev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_route_input_noref);

__weak int inet_addr_onlink(struct in_device *in_dev, __be32 a, __be32 b)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_addr_onlink);

__weak void rt_cache_flush(struct net *net)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rt_cache_flush);

/*
int call_netevent_notifiers(unsigned long val, void *v)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(call_netevent_notifiers);
*/

int proc_dointvec_userhz_jiffies(struct ctl_table *table, int write,
                 void *buffer, size_t *lenp, loff_t *ppos)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(proc_dointvec_userhz_jiffies);

__weak struct nlmsghdr *
__nlmsg_put(struct sk_buff *skb, u32 portid, u32 seq, int type, int len, int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__nlmsg_put);

__weak void rtnl_set_sk_err(struct net *net, u32 group, int error)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_set_sk_err);

__weak int rtnl_unicast(struct sk_buff *skb, struct net *net, u32 pid)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_unicast);

__weak void rtnl_register(int protocol, int msgtype,
		   rtnl_doit_func doit, rtnl_dumpit_func dumpit,
		   unsigned int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_register);

__weak void rtnl_notify(struct sk_buff *skb, struct net *net, u32 pid, u32 group,
		 struct nlmsghdr *nlh, gfp_t flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rtnl_notify);

__weak void __ip_select_ident(struct net *net, struct iphdr *iph, int segs)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__ip_select_ident);

__weak struct rtable *rt_dst_clone(struct net_device *dev, struct rtable *rt)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rt_dst_clone);

__weak int __init ip_rt_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_rt_init);

__weak void ip_local_error(struct sock *sk, int err, __be32 daddr, __be16 port, u32 info)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_local_error);

int __ip_options_echo(struct net *net, struct ip_options *dopt,
              struct sk_buff *skb, const struct ip_options *sopt)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__ip_options_echo);

__weak bool netlink_strict_get_check(struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(netlink_strict_get_check);

__weak __be32 inet_select_addr(const struct net_device *dev, __be32 dst, int scope)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_select_addr);

__weak __be32 inet_confirm_addr(struct net *net, struct in_device *in_dev,
			 __be32 dst, __be32 local, int scope)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_confirm_addr);

/*
void dst_dev_put(struct dst_entry *dst)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(dst_dev_put);
*/

__weak void rt_flush_dev(struct net_device *dev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(rt_flush_dev);

__weak struct in_device *inetdev_by_index(struct net *net, int ifindex)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inetdev_by_index);

__weak int register_inetaddr_notifier(struct notifier_block *nb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(register_inetaddr_notifier);

__weak struct in_ifaddr *inet_ifa_byprefix(struct in_device *in_dev, __be32 prefix,
				    __be32 mask)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_ifa_byprefix);

__weak struct in_ifaddr *inet_lookup_ifaddr_rcu(struct net *net, __be32 addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_lookup_ifaddr_rcu);

__weak int netlink_unicast(struct sock *ssk, struct sk_buff *skb,
		    u32 portid, int nonblock)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(netlink_unicast);

__weak unsigned int inet_addr_type_dev_table(struct net *net,
				      const struct net_device *dev,
				      __be32 addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_addr_type_dev_table);

__weak int fib_dump_info_fnhe(struct sk_buff *skb, struct netlink_callback *cb,
		       u32 table_id, struct fib_info *fi,
		       int *fa_index, int fa_start, unsigned int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(fib_dump_info_fnhe);

__weak void __init inet_initpeers(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_initpeers);

__weak struct net_device *__ip_dev_find(struct net *net, __be32 addr, bool devref)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__ip_dev_find);

__weak int devinet_ioctl(struct net *net, unsigned int cmd, struct ifreq *ifr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(devinet_ioctl);

__weak void in_dev_finish_destroy(struct in_device *idev)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(in_dev_finish_destroy);

__weak void __init devinet_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(devinet_init);

__weak int __init igmp_mc_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(igmp_mc_init);

/*
int dev_mc_del(struct net_device *dev, const unsigned char *addr)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(dev_mc_del);
*/

__weak int __inet_stream_connect(struct socket *sock, struct sockaddr *uaddr,
			  int addr_len, int flags, int is_sendmsg)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(__inet_stream_connect);

__weak void inet_sk_state_store(struct sock *sk, int newstate)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_sk_state_store);

void put_cmsg_scm_timestamping64(struct msghdr *msg, struct scm_timestamping_internal *tss_internal)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_cmsg_scm_timestamping64);

__weak void __init tcp_v4_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_v4_init);

/*
void tcp_fastopen_active_disable_ofo_check(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_fastopen_active_disable_ofo_check);
*/

__weak int tcp_connect(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_connect);

/*
void inet_csk_clear_xmit_timers(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_csk_clear_xmit_timers);
*/

__weak struct sk_buff *tcp_get_timestamping_opt_stats(const struct sock *sk,
					       const struct sk_buff *orig_skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_get_timestamping_opt_stats);

void put_cmsg_scm_timestamping(struct msghdr *msg, struct scm_timestamping_internal *tss_internal)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(put_cmsg_scm_timestamping);

__weak void sock_edemux(struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sock_edemux);

__weak void inet_put_port(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_put_port);

__weak void inet_hashinfo_init(struct inet_hashinfo *h)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_hashinfo_init);

__weak void inet_unhash(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_unhash);

__weak void inet_sk_set_state(struct sock *sk, int state)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_sk_set_state);

/*
void inet_csk_destroy_sock(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_csk_destroy_sock);
*/

int reuseport_alloc(struct sock *sk, bool bind_inany)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(reuseport_alloc);

/*
bool inet_rcv_saddr_any(const struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_rcv_saddr_any);
*/

int reuseport_add_sock(struct sock *sk, struct sock *sk2, bool bind_inany)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(reuseport_add_sock);

/*
bool inet_rcv_saddr_equal(const struct sock *sk, const struct sock *sk2,
              bool match_wildcard)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_rcv_saddr_equal);
*/

struct sock *reuseport_select_sock(struct sock *sk,
                   u32 hash,
                   struct sk_buff *skb,
                   int hdr_len)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(reuseport_select_sock);

/*
void inet_get_local_port_range(struct net *net, int *low, int *high)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_get_local_port_range);
*/

__weak void __init inet_hashinfo2_init(struct inet_hashinfo *h, const char *name,
				unsigned long numentries, int scale,
				unsigned long low_limit,
				unsigned long high_limit)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_hashinfo2_init);

void inet_twsk_deschedule_put(struct inet_timewait_sock *tw)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_twsk_deschedule_put);

__weak void sock_gen_put(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(sock_gen_put);

__weak int inet_ehash_locks_alloc(struct inet_hashinfo *hashinfo)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_ehash_locks_alloc);

__weak unsigned int tcp_sync_mss(struct sock *sk, u32 pmtu)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_sync_mss);

__weak void __init tcp_tasklet_init(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_tasklet_init);

__weak void tcp_send_active_reset(struct sock *sk, gfp_t priority)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_send_active_reset);

__weak void tcp_write_queue_purge(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_write_queue_purge);

/*
 * Pressure flag: try to collapse.
 * Technical note: it is used by multiple contexts non atomically.
 * All the __sk_mem_schedule() is of this nature: accounting
 * is strict, actions are advisory and have some latency.
 */
unsigned long tcp_memory_pressure __read_mostly;
EXPORT_SYMBOL_GPL(tcp_memory_pressure);

DEFINE_STATIC_KEY_FALSE(tcp_tx_skb_cache_key);
EXPORT_SYMBOL(tcp_tx_skb_cache_key);

DEFINE_STATIC_KEY_FALSE(tcp_tx_delay_enabled);
EXPORT_SYMBOL(tcp_tx_delay_enabled);

__weak struct sk_buff *sk_stream_alloc_skb(struct sock *sk, int size, gfp_t gfp,
				    bool force_schedule)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(sk_stream_alloc_skb);

void tcp_initialize_rcv_mss(struct sock *sk)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_initialize_rcv_mss);

void tcp_clear_retrans(struct tcp_sock *tp)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_clear_retrans);

__weak void tcp_wfree(struct sk_buff *skb)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(tcp_wfree);

__weak void inet_register_protosw(struct inet_protosw *p)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_register_protosw);

__weak void __init udplite4_register(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(udplite4_register);

__weak void inet_putpeer(struct inet_peer *p)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_putpeer);

__weak struct inet_peer *inet_getpeer(struct inet_peer_base *base,
			       const struct inetpeer_addr *daddr,
			       int create)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(inet_getpeer);

__weak void ipv4_redirect(struct sk_buff *skb, struct net *net,
		   int oif, u8 protocol)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ipv4_redirect);

__weak int ip_append_data(struct sock *sk, struct flowi4 *fl4,
		   int getfrag(void *from, char *to, int offset, int len,
			       int odd, struct sk_buff *skb),
		   void *from, int length, int transhdrlen,
		   struct ipcm_cookie *ipc, struct rtable **rtp,
		   unsigned int flags)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(ip_append_data);

__weak void raw_icmp_error(struct sk_buff *skb, int protocol, u32 info)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(raw_icmp_error);

__weak void ping_err(struct sk_buff *skb, int offset, u32 info)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(ping_err);

__weak __be32 inet_current_timestamp(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(inet_current_timestamp);

/* An array of errno for error messages from dest unreach. */
/* RFC 1122: 3.2.2.1 States that NET_UNREACH, HOST_UNREACH and SR_FAILED MUST be considered 'transient errs'. */

const struct icmp_err icmp_err_convert[] = {
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNREACH */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_UNREACH */
		.fatal = 0,
	},
	{
		.errno = ENOPROTOOPT	/* ICMP_PROT_UNREACH */,
		.fatal = 1,
	},
	{
		.errno = ECONNREFUSED,	/* ICMP_PORT_UNREACH */
		.fatal = 1,
	},
	{
		.errno = EMSGSIZE,	/* ICMP_FRAG_NEEDED */
		.fatal = 0,
	},
	{
		.errno = EOPNOTSUPP,	/* ICMP_SR_FAILED */
		.fatal = 0,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNKNOWN */
		.fatal = 1,
	},
	{
		.errno = EHOSTDOWN,	/* ICMP_HOST_UNKNOWN */
		.fatal = 1,
	},
	{
		.errno = ENONET,	/* ICMP_HOST_ISOLATED */
		.fatal = 1,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_ANO	*/
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_ANO */
		.fatal = 1,
	},
	{
		.errno = ENETUNREACH,	/* ICMP_NET_UNR_TOS */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_HOST_UNR_TOS */
		.fatal = 0,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PKT_FILTERED */
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PREC_VIOLATION */
		.fatal = 1,
	},
	{
		.errno = EHOSTUNREACH,	/* ICMP_PREC_CUTOFF */
		.fatal = 1,
	},
};
EXPORT_SYMBOL(icmp_err_convert);

__weak int __init icmp_init(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(icmp_init);

__weak int icmp_rcv(struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(icmp_rcv);

__weak int icmp_err(struct sk_buff *skb, u32 info)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(icmp_err);

__weak int __init ip_misc_proc_init(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_misc_proc_init);

int __ipv6_addr_type(const struct in6_addr *addr)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__ipv6_addr_type);

__sum16 csum_ipv6_magic(const struct in6_addr *saddr,
            const struct in6_addr *daddr,
            __u32 len, __u8 proto, __wsum csum)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(csum_ipv6_magic);

/*
struct metadata_dst *metadata_dst_alloc(u8 optslen, enum metadata_type type,
                    gfp_t flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(metadata_dst_alloc);
*/

__weak struct metadata_dst *iptunnel_metadata_reply(struct metadata_dst *md,
					     gfp_t flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(iptunnel_metadata_reply);

__weak void tcp_init_xmit_timers(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_init_xmit_timers);

void tcp_time_wait(struct sock *sk, int state, int timeo)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_time_wait);

/*
void inet_csk_reset_keepalive_timer(struct sock *sk, unsigned long len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(inet_csk_reset_keepalive_timer);
*/

__weak void tcp_delack_timer_handler(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_delack_timer_handler);

__weak void tcp_write_timer_handler(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_write_timer_handler);

__weak bool inet_ehash_insert(struct sock *sk, struct sock *osk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(inet_ehash_insert);

__weak void inet_bind_hash(struct sock *sk, struct inet_bind_bucket *tb,
		    const unsigned short snum)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(inet_bind_hash);

__weak struct inet_bind_bucket *inet_bind_bucket_create(struct kmem_cache *cachep,
						 struct net *net,
						 struct inet_bind_hashbucket *head,
						 const unsigned short snum,
						 int l3mdev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(inet_bind_bucket_create);

void tcp_fin(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_fin);

void tcp_init_transfer(struct sock *sk, int bpf_op)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_init_transfer);

__weak bool tcp_fastopen_cookie_check(struct sock *sk, u16 *mss,
			       struct tcp_fastopen_cookie *cookie)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(tcp_fastopen_cookie_check);

__weak __sum16 __skb_checksum_complete(struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__skb_checksum_complete);

__weak kuid_t sock_i_uid(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sock_i_uid);

__weak struct net_device *__dev_get_by_name(struct net *net, const char *name)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__dev_get_by_name);

__weak int call_fib_notifiers(struct net *net, enum fib_event_type event_type,
		       struct fib_notifier_info *info)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(call_fib_notifiers);

__weak struct net_device *__dev_get_by_index(struct net *net, int ifindex)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__dev_get_by_index);

/*
struct net_device *blackhole_netdev;
EXPORT_SYMBOL(blackhole_netdev);
*/

__weak void bpf_warn_invalid_xdp_action(u32 act)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(bpf_warn_invalid_xdp_action);

/*
void xdp_rxq_info_unreg(struct xdp_rxq_info *xdp_rxq)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(xdp_rxq_info_unreg);

void netif_carrier_off(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netif_carrier_off);

void netif_carrier_on(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netif_carrier_on);
*/

/*
int xdp_rxq_info_reg(struct xdp_rxq_info *xdp_rxq,
             struct net_device *dev, u32 queue_index)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(xdp_rxq_info_reg);
*/

struct flow_dissector flow_keys_basic_dissector __read_mostly;
EXPORT_SYMBOL(flow_keys_basic_dissector);

__weak __be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(eth_type_trans);

__weak ssize_t sysfs_format_mac(char *buf, const unsigned char *addr, int len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sysfs_format_mac);

bool __skb_flow_dissect(const struct net *net,
            const struct sk_buff *skb,
            struct flow_dissector *flow_dissector,
            void *target_container,
            void *data, __be16 proto, int nhoff, int hlen,
            unsigned int flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__skb_flow_dissect);

__weak int dev_pre_changeaddr_notify(struct net_device *dev, const char *addr,
			      struct netlink_ext_ack *extack)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_pre_changeaddr_notify);

__weak void __dev_set_rx_mode(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__dev_set_rx_mode);

__weak int call_netdevice_notifiers(unsigned long val, struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(call_netdevice_notifiers);

__weak int ethtool_check_ops(const struct ethtool_ops *ops)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ethtool_check_ops);

__weak int dev_ethtool(struct net *net, struct ifreq *ifr)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_ethtool);

__weak int __ethtool_get_link_ksettings(struct net_device *dev,
				 struct ethtool_link_ksettings *link_ksettings)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__ethtool_get_link_ksettings);

__weak void linkwatch_fire_event(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(linkwatch_fire_event);

__weak int dev_get_iflink(const struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_get_iflink);

__weak void netdev_state_change(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_state_change);

__weak void linkwatch_init_dev(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(linkwatch_init_dev);

__weak void linkwatch_run_queue(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(linkwatch_run_queue);

__weak void linkwatch_forget_dev(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(linkwatch_forget_dev);

/*
void dev_activate(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_activate);

void dev_deactivate(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_deactivate);
*/

__weak void __netif_schedule(struct Qdisc *q)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__netif_schedule);

__weak struct rtnl_link_stats64 *dev_get_stats(struct net_device *dev,
					struct rtnl_link_stats64 *storage)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_get_stats);

__weak bool refcount_dec_and_rtnl_lock(refcount_t *r)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(refcount_dec_and_rtnl_lock);

__weak void netif_schedule_queue(struct netdev_queue *txq)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netif_schedule_queue);

int dev_tx_weight __read_mostly = 64;
EXPORT_SYMBOL(dev_tx_weight);

__weak struct sk_buff *validate_xmit_skb_list(struct sk_buff *skb, struct net_device *dev, bool *again)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(validate_xmit_skb_list);

__weak void kfree_skb_list(struct sk_buff *segs)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(kfree_skb_list);

__weak void synchronize_net(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(synchronize_net);

__weak const char *netdev_drivername(const struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(netdev_drivername);

__weak struct sk_buff *dev_hard_start_xmit(struct sk_buff *first, struct net_device *dev,
				    struct netdev_queue *txq, int *ret)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(dev_hard_start_xmit);

__weak char *dentry_path(struct dentry *dentry, char *buf, int buflen)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dentry_path);

__weak char *__d_path(const struct path *path,
	       const struct path *root,
	       char *buf, int buflen)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(__d_path);

__weak struct dst_metrics *ip_fib_metrics_init(struct net *net, struct nlattr *fc_mx,
					int fc_mx_len,
					struct netlink_ext_ack *extack)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(ip_fib_metrics_init);

__weak enum hrtimer_restart it_real_fn(struct hrtimer *timer)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(it_real_fn);

int ip_options_compile(struct net *net,
               struct ip_options *opt, struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_options_compile);

int ip_options_rcv_srr(struct sk_buff *skb, struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_options_rcv_srr);

__weak void ip_list_rcv(struct list_head *head, struct packet_type *pt,
		 struct net_device *orig_dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_list_rcv);

__weak int ip_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt,
	   struct net_device *orig_dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_rcv);

__weak int ip_local_deliver(struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_local_deliver);

__weak void ip_protocol_deliver_rcu(struct net *net, struct sk_buff *skb, int protocol)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_protocol_deliver_rcu);

__weak struct metadata_dst *metadata_dst_alloc(u8 optslen, enum metadata_type type,
					gfp_t flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(metadata_dst_alloc);

__weak int eth_mac_addr(struct net_device *dev, void *p)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(eth_mac_addr);

__weak void skb_tstamp_tx(struct sk_buff *orig_skb,
		   struct skb_shared_hwtstamps *hwtstamps)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(skb_tstamp_tx);

__weak int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(skb_copy_bits);

__weak struct net_device *alloc_netdev_mqs(int sizeof_priv, const char *name,
		unsigned char name_assign_type,
		void (*setup)(struct net_device *),
		unsigned int txqs, unsigned int rxqs)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(alloc_netdev_mqs);

__weak struct packet_offload *gro_find_complete_by_type(__be16 type)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(gro_find_complete_by_type);

__weak void *skb_push(struct sk_buff *skb, unsigned int len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(skb_push);

__weak void *__pskb_pull_tail(struct sk_buff *skb, int delta)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__pskb_pull_tail);

__weak int __init netdev_boot_setup(char *str)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_boot_setup);

__weak void dev_add_offload(struct packet_offload *po)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_add_offload);

__weak struct packet_offload *gro_find_receive_by_type(__be16 type)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(gro_find_receive_by_type);

__weak int register_netdev(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(register_netdev);

__weak void free_netdev(struct net_device *dev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(free_netdev);

__weak int netif_rx(struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netif_rx);

__weak int ethtool_op_get_ts_info(struct net_device *dev, struct ethtool_ts_info *info)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ethtool_op_get_ts_info);

__weak int ip_ra_control(struct sock *sk, unsigned char on,
		  void (*destructor)(struct sock *))
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(ip_ra_control);

DEFINE_STATIC_KEY_FALSE(bpf_sk_lookup_enabled);
EXPORT_SYMBOL(bpf_sk_lookup_enabled);

__weak int copy_bpf_fprog_from_user(struct sock_fprog *dst, sockptr_t src, int len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(copy_bpf_fprog_from_user);

__weak int xdp_do_generic_redirect(struct net_device *dev, struct sk_buff *skb,
			    struct xdp_buff *xdp, struct bpf_prog *xdp_prog)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(xdp_do_generic_redirect);

__weak int sk_attach_bpf(u32 ufd, struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_attach_bpf);

__weak void bpf_prog_change_xdp(struct bpf_prog *prev_prog, struct bpf_prog *prog)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(bpf_prog_change_xdp);

__weak bool sk_filter_charge(struct sock *sk, struct sk_filter *fp)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_filter_charge);

__weak int sk_detach_filter(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(sk_detach_filter);

__weak void sk_filter_uncharge(struct sock *sk, struct sk_filter *fp)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_filter_uncharge);

u64 sock_gen_cookie(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sock_gen_cookie);

__weak int sk_attach_filter(struct sock_fprog *fprog, struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(sk_attach_filter);

__weak int sk_reuseport_attach_filter(struct sock_fprog *fprog, struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_reuseport_attach_filter);

void __skb_get_hash(struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__skb_get_hash);

__weak int sk_reuseport_attach_bpf(u32 ufd, struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_reuseport_attach_bpf);

__weak int sk_get_filter(struct sock *sk, struct sock_filter __user *ubuf,
		  unsigned int len)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sk_get_filter);

__weak void __init skb_init(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(skb_init);

__weak struct sk_buff *sock_dequeue_err_skb(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sock_dequeue_err_skb);

__weak struct sk_buff *alloc_skb_with_frags(unsigned long header_len,
				     unsigned long data_len,
				     int max_page_order,
				     int *errcode,
				     gfp_t gfp_mask)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(alloc_skb_with_frags);

__weak void net_disable_timestamp(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(net_disable_timestamp);

__weak struct net_device *dev_get_by_napi_id(unsigned int napi_id)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_get_by_napi_id);

__weak void net_enable_timestamp(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(net_enable_timestamp);

__weak void napi_busy_loop(unsigned int napi_id,
		    bool (*loop_end)(void *, unsigned long),
		    void *loop_end_arg)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(napi_busy_loop);

__weak struct net_device *dev_get_by_name_rcu(struct net *net, const char *name)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_get_by_name_rcu);

DEFINE_PER_CPU_ALIGNED(struct softnet_data, softnet_data);
EXPORT_PER_CPU_SYMBOL(softnet_data);

struct pipe_buf_operations;
const struct pipe_buf_operations nosteal_pipe_buf_ops;
EXPORT_SYMBOL(nosteal_pipe_buf_ops);

__weak int __zerocopy_sg_from_iter(struct sock *sk, struct sk_buff *skb,
			    struct iov_iter *from, size_t length)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__zerocopy_sg_from_iter);

__weak void __dev_kfree_skb_any(struct sk_buff *skb, enum skb_free_reason reason)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(__dev_kfree_skb_any);

__weak void netdev_rx_csum_fault(struct net_device *dev, struct sk_buff *skb)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_rx_csum_fault);

__weak int netdev_queue_update_kobjects(struct net_device *dev, int old_num, int new_num)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_queue_update_kobjects);

__weak int netdev_register_kobject(struct net_device *ndev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_register_kobject);

__weak int
net_rx_queue_update_kobjects(struct net_device *dev, int old_num, int new_num)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(net_rx_queue_update_kobjects);

__weak int __init dev_proc_init(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_proc_init);

__weak void netdev_unregister_kobject(struct net_device *ndev)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_unregister_kobject);

__weak int sysfs_rename_link_ns(struct kobject *kobj, struct kobject *targ,
			 const char *old, const char *new, const void *new_ns)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL_GPL(sysfs_rename_link_ns);

__weak int netdev_change_owner(struct net_device *ndev, const struct net *net_old,
			const struct net *net_new)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_change_owner);

__weak int __init netdev_kobject_init(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_kobject_init);

ssize_t generic_splice_sendpage(struct pipe_inode_info *pipe, struct file *out,
                loff_t *ppos, size_t len, unsigned int flags)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(generic_splice_sendpage);

__weak int netdev_get_name(struct net *net, char *name, int ifindex)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(netdev_get_name);

void sock_diag_broadcast_destroy(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sock_diag_broadcast_destroy);

__weak __be16 skb_network_protocol(struct sk_buff *skb, int *depth)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(skb_network_protocol);

__weak int reuseport_detach_prog(struct sock *sk)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(reuseport_detach_prog);

int poll_select_set_timeout(struct timespec64 *to, time64_t sec, long nsec)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(poll_select_set_timeout);

__weak int dev_ifconf(struct net *net, struct ifconf *ifc, int size)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_ifconf);

ssize_t splice_to_pipe(struct pipe_inode_info *pipe,
               struct splice_pipe_desc *spd)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(splice_to_pipe);

__weak int dev_ioctl(struct net *net, unsigned int cmd, struct ifreq *ifr, bool *need_copyout)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(dev_ioctl);

__weak int sysfs_rename_dir_ns(struct kobject *kobj, const char *new_name,
			const void *new_ns)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(sysfs_rename_dir_ns);

__weak int copy_thread(unsigned long clone_flags, unsigned long usp, unsigned long arg,
		struct task_struct *p, unsigned long tls)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(copy_thread);

__weak void show_regs(struct pt_regs *regs)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(show_regs);

__weak void start_thread(struct pt_regs *regs, unsigned long pc,
	unsigned long sp)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(start_thread);

__weak void flush_thread(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(flush_thread);

__weak void __init parse_early_param(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(parse_early_param);

__weak int access_process_vm(struct task_struct *tsk, unsigned long addr,
		void *buf, int len, unsigned int gup_flags)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL_GPL(access_process_vm);

#ifdef CONFIG_FPU
bool has_fpu __read_mostly;
EXPORT_SYMBOL(has_fpu);
#endif

__weak void *kmalloc_node(size_t size, gfp_t flags, int node)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(kmalloc_node);

__weak void *kmalloc(size_t size, gfp_t flags)
{
    booter_panic("No impl 'slub'.");
}
EXPORT_SYMBOL(kmalloc);

__weak const struct file_operations *get_def_blk_fops(void)
{
    booter_panic("No impl.");
}
EXPORT_SYMBOL(get_def_blk_fops);

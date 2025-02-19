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

/*
void kobject_del(struct kobject *kobj)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(kobject_del);
*/

struct kobject *kernel_kobj;
EXPORT_SYMBOL_GPL(kernel_kobj);

void module_put(struct module *module)
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

struct ucounts *inc_ucount(struct user_namespace *ns, kuid_t uid,
               enum ucount_type type)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(inc_ucount);
void dec_ucount(struct ucounts *ucounts, enum ucount_type type)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(dec_ucount);
void proc_free_inum(unsigned int inum)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(proc_free_inum);
int proc_alloc_inum(unsigned int *inum)
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

struct dentry *d_make_root(struct inode *root_inode)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(d_make_root);

int simple_readpage(struct file *file, struct page *page)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_readpage);

int simple_write_begin(struct file *file, struct address_space *mapping,
            loff_t pos, unsigned len, unsigned flags,
            struct page **pagep, void **fsdata)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(simple_write_begin);

int simple_write_end(struct file *file, struct address_space *mapping,
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

struct proc_dir_entry *proc_create_single_data(const char *name, umode_t mode,
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

void lockref_get(struct lockref *lockref)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(lockref_get);

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


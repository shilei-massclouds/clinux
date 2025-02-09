// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
#include <linux/kobject.h>
#include "../../booter/src/booter.h"

/*
 * Low level drivers may need that to know if they can schedule in
 * their unblank() callback or not. So let's export it.
 */
int oops_in_progress;
EXPORT_SYMBOL(oops_in_progress);

#if defined(CONFIG_PROVE_LOCKING) || defined(CONFIG_DEBUG_ATOMIC_SLEEP)
void __might_fault(const char *file, int line)
{
    sbi_puts("unimplemented __might_fault!");
    sbi_shutdown();
}
EXPORT_SYMBOL(__might_fault);
#endif

int cl_lib_init(void)
{
    sbi_puts("module[lib]: init begin ...\n");
    sbi_puts("module[lib]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_lib_init);

void print_worker_info(const char *log_lvl, struct task_struct *task)
{
    booter_panic("No impl 'print_worker_info'.");
}

int __kernel_text_address(unsigned long addr)
{
    booter_panic("No impl '__kernel_text_address'.");
}

int in_sched_functions(unsigned long addr)
{
  return in_lock_functions(addr) ||
      (addr >= (unsigned long)__sched_text_start
      && addr < (unsigned long)__sched_text_end);
}

const char *print_tainted(void)
{
    booter_panic("No impl 'print_tainted'.");
}

bool ns_capable(struct user_namespace *ns, int cap)
{
    booter_panic("No impl 'ns_capable'.");
}
EXPORT_SYMBOL(ns_capable);

void key_put(struct key *key)
{
    booter_panic("No impl 'key_put'.");
}

void sysfs_remove_groups(struct kobject *kobj,
				       const struct attribute_group **groups)
{
    booter_panic("No impl in 'lib'.");
}

void kfree_const(const void *x)
{
    booter_panic("No impl in 'lib'.");
}

void kernfs_put(struct kernfs_node *kn)
{
    booter_panic("No impl in 'lib'.");
}

int kobject_uevent(struct kobject *kobj, enum kobject_action action)
{
    booter_panic("No impl in 'lib'.");
}

void sysfs_remove_dir(struct kobject *kobj)
{
    booter_panic("No impl in 'lib'.");
}

int wake_up_process(struct task_struct *p)
{
    booter_panic("No impl in 'lib'.");
}
EXPORT_SYMBOL(wake_up_process);

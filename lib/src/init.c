// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/sched/debug.h>
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
EXPORT_SYMBOL(print_worker_info);

/*
int __kernel_text_address(unsigned long addr)
{
    booter_panic("No impl '__kernel_text_address'.");
}
*/

int in_sched_functions(unsigned long addr)
{
  return in_lock_functions(addr) ||
      (addr >= (unsigned long)__sched_text_start
      && addr < (unsigned long)__sched_text_end);
}
EXPORT_SYMBOL(in_sched_functions);

const char *print_tainted(void)
{
    booter_panic("No impl 'print_tainted'.");
}
EXPORT_SYMBOL(print_tainted);

bool ns_capable(struct user_namespace *ns, int cap)
{
    booter_panic("No impl 'ns_capable'.");
}
EXPORT_SYMBOL(ns_capable);

void key_put(struct key *key)
{
    booter_panic("No impl 'key_put'.");
}
EXPORT_SYMBOL(key_put);

void __warn_printk(const char *fmt, ...)
{
    sbi_puts("[RAW_WARN] ");
    sbi_puts(printk_skip_level(fmt));
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/ptrace.h>
#include <linux/compiler_attributes.h>
#include <linux/kdebug.h>
#include <linux/irq.h>
#include <asm/unistd.h>
#include "../../booter/src/booter.h"

int
cl_traps_init(void)
{
    sbi_puts("module[traps]: init begin ...\n");
    sbi_puts("module[traps]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_traps_init);

asmlinkage void do_page_fault(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}

__visible int do_syscall_trace_enter(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}

int fixup_exception(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}

// NOTE: implement it!!!!!!!!!!!!!!!!
void *sys_call_table[__NR_syscalls];

// NOTE: implement it!!!!!!!!!!!!!!!!
int panic_on_oops = CONFIG_PANIC_ON_OOPS_VALUE;

void bust_spinlocks(int yes)
{
    booter_panic("No impl!\n");
}
void oops_enter(void)
{
    booter_panic("No impl!\n");
}
int __printk_ratelimit(const char *func)
{
    booter_panic("No impl!\n");
}
void
show_regs(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
bool unhandled_signal(struct task_struct *tsk, int sig)
{
    booter_panic("No impl!\n");
}
void __noreturn do_exit(long code)
{
    booter_panic("No impl!\n");
    do {} while(1);
}
void oops_exit(void)
{
    booter_panic("No impl!\n");
}
/*
int notrace notify_die(enum die_val val, const char *str,
           struct pt_regs *regs, long err, int trap, int sig)
{
    booter_panic("No impl!\n");
}
*/
asmlinkage __visible void schedule_tail(struct task_struct *prev)
{
    booter_panic("No impl!\n");
}
int force_sig_fault(int sig, int code, void __user *addr
    ___ARCH_SI_TRAPNO(int trapno)
    ___ARCH_SI_IA64(int imm, unsigned int flags, unsigned long isr))
{
    booter_panic("No impl!\n");
}
void do_notify_resume(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
asmlinkage long sys_ni_syscall(void)
{
    booter_panic("No impl!\n");
}
void print_vma_addr(char *prefix, unsigned long rip)
{
    booter_panic("No impl!\n");
}
enum bug_trap_type report_bug(unsigned long bug_addr,
                        struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
asmlinkage void do_syscall_trace_exit(void)
{
    booter_panic("No impl!\n");
}

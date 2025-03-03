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

__visible int do_syscall_trace_enter(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}

/*
void do_notify_resume(struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
*/
asmlinkage long sys_ni_syscall(void)
{
    booter_panic("No impl!\n");
}
/*
enum bug_trap_type report_bug(unsigned long bug_addr,
                        struct pt_regs *regs)
{
    booter_panic("No impl!\n");
}
*/
asmlinkage void do_syscall_trace_exit(void)
{
    booter_panic("No impl!\n");
}

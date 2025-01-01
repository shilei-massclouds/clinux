// SPDX-License-Identifier: GPL-2.0-or-later

#include <csr.h>
#include <sched.h>
#include <ptrace.h>
#include <string.h>
#include <processor.h>

extern void ret_from_kernel_thread(void);

register unsigned long gp_in_global __asm__("gp");

int copy_thread(unsigned long clone_flags,
                unsigned long usp, unsigned long arg,
                struct task_struct *p, unsigned long tls)
{
    struct pt_regs *childregs = task_pt_regs(p);

    if (unlikely(p->flags & PF_KTHREAD)) {
        /* Kernel thread */
        memset(childregs, 0, sizeof(struct pt_regs));
        childregs->gp = gp_in_global;
        /* Supervisor/Machine, irqs on: */
        childregs->status = SR_PP | SR_PIE;

        p->thread.ra = (unsigned long)ret_from_kernel_thread;
        p->thread.s[0] = usp; /* fn */
        p->thread.s[1] = arg;
    } else {
        panic("User fork!");
    }

    p->thread.sp = (unsigned long)childregs; /* kernel sp */
    printk("%s: !\n", __func__);
    return 0;
}

int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src)
{
    *dst = *src;
    return 0;
}

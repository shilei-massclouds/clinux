/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _ASM_GENERIC_IRQ_REGS_H
#define _ASM_GENERIC_IRQ_REGS_H

#include <types.h>
#include <ptrace.h>
#include <irqdomain.h>

struct pt_regs *__irq_regs = NULL;

static inline struct pt_regs *
set_irq_regs(struct pt_regs *new_regs)
{
    struct pt_regs *old_regs;

    old_regs = __irq_regs;
    __irq_regs = new_regs;
    return old_regs;
}

#endif /* _ASM_GENERIC_IRQ_REGS_H */

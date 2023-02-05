/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_SWITCH_TO_H
#define _ASM_RISCV_SWITCH_TO_H

static inline void __fstate_clean(struct pt_regs *regs)
{
    regs->status = (regs->status & ~SR_FS) | SR_FS_CLEAN;
}

static inline
void fstate_restore(struct task_struct *task, struct pt_regs *regs)
{
    if ((regs->status & SR_FS) != SR_FS_OFF) {
        //__fstate_restore(task);
        __fstate_clean(regs);
    }
}

#endif /* _ASM_RISCV_SWITCH_TO_H */

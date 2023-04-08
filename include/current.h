/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_CURRENT_H
#define _ASM_RISCV_CURRENT_H

#ifndef __ASSEMBLY__

#include <sched.h>

extern struct task_struct *saved_riscv_tp;

register struct task_struct *riscv_current_is_tp __asm__("tp");

static __always_inline struct task_struct *get_current(void)
{
    return saved_riscv_tp ? saved_riscv_tp : riscv_current_is_tp;
}

#define current get_current()

static __always_inline void save_current(void)
{
    saved_riscv_tp = riscv_current_is_tp;
}

#define save_current save_current

static __always_inline struct task_struct *get_saved_current(void)
{
    return saved_riscv_tp;
}

#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_CURRENT_H */

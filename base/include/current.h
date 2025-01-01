/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_CURRENT_H
#define _ASM_RISCV_CURRENT_H

#ifndef __ASSEMBLY__

#include <sched.h>

register struct task_struct *riscv_current_is_tp __asm__("tp");

static __always_inline struct task_struct *get_current(void)
{
    return riscv_current_is_tp;
}

#define current get_current()

#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_CURRENT_H */

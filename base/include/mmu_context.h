/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_MMU_CONTEXT_H
#define _ASM_RISCV_MMU_CONTEXT_H

#include <mm.h>
#include <sched.h>
#include <mm_types.h>

void switch_mm(struct mm_struct *prev, struct mm_struct *next,
               struct task_struct *task);

static inline void
activate_mm(struct mm_struct *prev, struct mm_struct *next)
{
    switch_mm(prev, next, NULL);
}

static inline void
deactivate_mm(struct task_struct *task, struct mm_struct *mm)
{
}

#endif /* _ASM_RISCV_MMU_CONTEXT_H */

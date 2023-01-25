/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_PROCESSOR_H
#define _ASM_RISCV_PROCESSOR_H

#define STACK_TOP       TASK_SIZE
#define STACK_TOP_MAX   STACK_TOP
#define STACK_ALIGN     16

#ifndef __ASSEMBLY__

#include <task_stack.h>

#define task_pt_regs(tsk) \
    ((struct pt_regs *)(task_stack_page(tsk) + THREAD_SIZE \
                        - ALIGN(sizeof(struct pt_regs), STACK_ALIGN)))

#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_PROCESSOR_H */

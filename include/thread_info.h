/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_THREAD_INFO_H
#define _ASM_RISCV_THREAD_INFO_H

#include <page.h>

#define THREAD_SIZE_ORDER   CONFIG_THREAD_SIZE_ORDER
#define THREAD_SIZE         (PAGE_SIZE << THREAD_SIZE_ORDER)

#define TASK_TI_FLAGS           0
#define TASK_TI_PREEMPT_COUNT   8
#define TASK_TI_KERNEL_SP       24
#define TASK_TI_USER_SP         32
#define TASK_TI_CPU             40

#define THREADINFO_GFP      (GFP_KERNEL_ACCOUNT | __GFP_ZERO)

#define TIF_NOTIFY_RESUME   1   /* callback before returning to user */
#define TIF_SIGPENDING      2   /* signal pending */
#define TIF_NEED_RESCHED    3   /* rescheduling necessary */

#define _TIF_NOTIFY_RESUME  (1 << TIF_NOTIFY_RESUME)
#define _TIF_SIGPENDING     (1 << TIF_SIGPENDING)
#define _TIF_NEED_RESCHED   (1 << TIF_NEED_RESCHED)

#define _TIF_WORK_MASK \
    (_TIF_NOTIFY_RESUME | _TIF_SIGPENDING | _TIF_NEED_RESCHED)

#ifndef __ASSEMBLY__

typedef struct {
    unsigned long seg;
} mm_segment_t;

struct thread_info {
    unsigned long   flags;          /* low level flags */
    int             preempt_count;  /* 0=>preemptible, <0=>BUG */
    mm_segment_t    addr_limit;
    /*
     * These stack pointers are overwritten on every system call or
     * exception.  SP is also saved to the stack it can be recovered when
     * overwritten.
     */
    long            kernel_sp;      /* Kernel stack pointer */
    long            user_sp;        /* User stack pointer */
    int             cpu;
};

/*
 * The fs value determines whether argument validity checking should be
 * performed or not.  If get_fs() == USER_DS, checking is performed, with
 * get_fs() == KERNEL_DS, checking is bypassed.
 *
 * For historical reasons, these macros are grossly misnamed.
 */

#define MAKE_MM_SEG(s)  ((mm_segment_t) { (s) })

#define KERNEL_DS   MAKE_MM_SEG(~0UL)
#define USER_DS     MAKE_MM_SEG(TASK_SIZE)

/*
 * macros/functions for gaining access to the thread information structure
 *
 * preempt_count needs to be 1 initially, until the scheduler is functional.
 */
#define INIT_THREAD_INFO(tsk)           \
{                       \
    .flags      = 0,            \
    .addr_limit = KERNEL_DS,        \
}

#include <current.h>
#define current_thread_info() ((struct thread_info *)current)

#endif /* !__ASSEMBLY__ */

#endif /* _ASM_RISCV_THREAD_INFO_H */

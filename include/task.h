/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_TASK_H
#define _LINUX_SCHED_TASK_H

#include <types.h>

#define arch_task_struct_size (sizeof(struct task_struct))

struct kernel_clone_args {
    u64 flags;
    unsigned long stack;
    unsigned long stack_size;
    unsigned long tls;
};

static inline void
arch_thread_struct_whitelist(unsigned long *offset, unsigned long *size)
{
    *offset = 0;
    /* Handle dynamically sized thread_struct. */
    *size = arch_task_struct_size - offsetof(struct task_struct, thread);
}

#endif /* _LINUX_SCHED_TASK_H */

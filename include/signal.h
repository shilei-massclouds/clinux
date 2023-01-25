/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_SIGNAL_H
#define _LINUX_SCHED_SIGNAL_H

#include <current.h>
#include <resource.h>

#define SIGILL      4

#define ILL_ILLTRP  4   /* illegal trap */

/*
 * SIGSEGV si_codes
 */
#define SEGV_MAPERR 1   /* address not mapped to object */
#define SEGV_ACCERR 2   /* invalid permissions for mapped object */

struct signal_struct {
    struct rlimit rlim[RLIM_NLIMITS];
};

static inline unsigned long
task_rlimit(const struct task_struct *task, unsigned int limit)
{
    return READ_ONCE(task->signal->rlim[limit].rlim_cur);
}

static inline unsigned long rlimit(unsigned int limit)
{
    return task_rlimit(current, limit);
}

#endif /* _LINUX_SCHED_SIGNAL_H */

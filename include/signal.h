/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_SIGNAL_H
#define _LINUX_SCHED_SIGNAL_H

#include <current.h>
#include <resource.h>

#define SIGHUP       1
#define SIGINT       2
#define SIGQUIT      3
#define SIGILL       4
#define SIGTRAP      5
#define SIGABRT      6
#define SIGIOT       6
#define SIGBUS       7
#define SIGFPE       8
#define SIGKILL      9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGPOLL     SIGIO
/*
#define SIGLOST     29
*/
#define SIGPWR      30
#define SIGSYS      31
#define SIGUNUSED   31

/* These should not be considered constants from userland.  */
#define SIGRTMIN    32

/*
 * SIGILL si_codes
 */
#define ILL_ILLOPC  1   /* illegal opcode */
#define ILL_ILLOPN  2   /* illegal operand */
#define ILL_ILLADR  3   /* illegal addressing mode */
#define ILL_ILLTRP  4   /* illegal trap */
#define ILL_PRVOPC  5   /* privileged opcode */
#define ILL_PRVREG  6   /* privileged register */
#define ILL_COPROC  7   /* coprocessor error */
#define ILL_BADSTK  8   /* internal stack error */
#define ILL_BADIADDR    9   /* unimplemented instruction address */
#define __ILL_BREAK 10  /* illegal break */
#define __ILL_BNDMOD    11  /* bundle-update (modification) in progress */
#define NSIGILL     11

/*
 * SIGBUS si_codes
 */
#define BUS_ADRALN  1   /* invalid address alignment */
#define BUS_ADRERR  2   /* non-existent physical address */
#define BUS_OBJERR  3   /* object specific hardware error */
/* hardware memory error consumed on a machine check: action required */
#define BUS_MCEERR_AR   4
/* hardware memory error detected in process but not consumed: action optional*/
#define BUS_MCEERR_AO   5
#define NSIGBUS     5


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

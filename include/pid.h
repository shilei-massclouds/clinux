/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PID_H
#define _LINUX_PID_H

#include <refcount.h>

enum pid_type
{
    PIDTYPE_PID,
    PIDTYPE_TGID,
    PIDTYPE_PGID,
    PIDTYPE_SID,
    PIDTYPE_MAX,
};

/*
 * struct upid is used to get the id of the struct pid, as it is
 * seen in particular namespace. Later the struct pid is found with
 * find_pid_ns() using the int nr and struct pid_namespace *ns.
 */

struct upid {
    int nr;
    struct pid_namespace *ns;
};

struct pid
{
    refcount_t count;
    unsigned int level;
#if 0
    spinlock_t lock;
    /* lists of tasks that use this pid */
    struct hlist_head tasks[PIDTYPE_MAX];
    struct hlist_head inodes;
    /* wait queue for pidfd notifications */
    wait_queue_head_t wait_pidfd;
    struct rcu_head rcu;
#endif
    struct upid numbers[1];
};

extern struct pid init_struct_pid;

/*
 * ns_of_pid() returns the pid namespace in which the specified pid was
 * allocated.
 *
 * NOTE:
 *  ns_of_pid() is expected to be called for a process (task) that has
 *  an attached 'struct pid' (see attach_pid(), detach_pid()) i.e @pid
 *  is expected to be non-NULL. If @pid is NULL, caller should handle
 *  the resulting NULL pid-ns.
 */
static inline struct pid_namespace *ns_of_pid(struct pid *pid)
{
    struct pid_namespace *ns = NULL;
    if (pid)
        ns = pid->numbers[pid->level].ns;
    return ns;
}

extern struct pid_namespace init_pid_ns;

#endif /* _LINUX_PID_H */

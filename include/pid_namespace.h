/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PID_NS_H
#define _LINUX_PID_NS_H

struct pid_namespace {
#if 0
    struct kref kref;
    struct idr idr;
    struct rcu_head rcu;
    unsigned int pid_allocated;
    struct task_struct *child_reaper;
    struct kmem_cache *pid_cachep;
#endif
    unsigned int level;
#if 0
    struct pid_namespace *parent;
    struct user_namespace *user_ns;
    struct ucounts *ucounts;
    int reboot; /* group exit code if this pidns was rebooted */
    struct ns_common ns;
#endif
};

#endif /* _LINUX_PID_NS_H */

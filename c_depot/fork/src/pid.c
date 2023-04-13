// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <syscalls.h>
#include <signal.h>

struct pid_namespace *task_active_pid_ns(struct task_struct *tsk)
{
    return ns_of_pid(task_pid(tsk));
}
EXPORT_SYMBOL(task_active_pid_ns);

pid_t pid_nr_ns(struct pid *pid, struct pid_namespace *ns)
{
    struct upid *upid;
    pid_t nr = 0;

    if (pid && ns->level <= pid->level) {
        upid = &pid->numbers[ns->level];
        if (upid->ns == ns)
            nr = upid->nr;
    }
    return nr;
}
EXPORT_SYMBOL(pid_nr_ns);

static struct pid **
task_pid_ptr(struct task_struct *task, enum pid_type type)
{
    return (type == PIDTYPE_PID) ?
        &task->thread_pid :
        &task->signal->pids[type];
}

pid_t
__task_pid_nr_ns(struct task_struct *task,
                 enum pid_type type,
                 struct pid_namespace *ns)
{
    pid_t nr = 0;

    //rcu_read_lock();
    if (!ns)
        ns = task_active_pid_ns(current);
    nr = pid_nr_ns(*task_pid_ptr(task, type), ns);
    //rcu_read_unlock();

    return nr;
}
EXPORT_SYMBOL(__task_pid_nr_ns);

long _do_getpid(void)
{
    return task_tgid_vnr(current);
}

void init_pid(void)
{
    do_getpid = _do_getpid;
}

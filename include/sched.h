/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <fs.h>
#include <thread_info.h>
#include <pid.h>
#include <pid_namespace.h>

#define MAX_NICE    19
#define MIN_NICE    -20
#define NICE_WIDTH  (MAX_NICE - MIN_NICE + 1)

#define MAX_USER_RT_PRIO    100
#define MAX_RT_PRIO MAX_USER_RT_PRIO

#define MAX_PRIO    (MAX_RT_PRIO + NICE_WIDTH)

/*
 * cloning flags:
 */
#define CSIGNAL     0x000000ff  /* signal mask to be sent at exit */
#define CLONE_VM    0x00000100  /* set if VM shared between processes */
#define CLONE_FS    0x00000200  /* set if fs info shared between processes */

#define CLONE_UNTRACED  0x00800000  /* set if the tracing process can't force CLONE_PTRACE on this clone */

/* Used in tsk->state: */
#define TASK_RUNNING            0x0000
#define TASK_INTERRUPTIBLE      0x0001
#define TASK_UNINTERRUPTIBLE    0x0002

#define TASK_NEW    0x0800

#define PF_IDLE     0x00000002  /* I am an IDLE thread */
#define PF_KTHREAD  0x00200000  /* I am a kernel thread */

#define cpu_rq()    (&runqueue)
#define task_rq(p)  cpu_rq()

#define TASK_ON_RQ_QUEUED   1

#define ENQUEUE_WAKEUP      0x01
#define ENQUEUE_NOCLOCK     0x08

#define SCHED_FIXEDPOINT_SHIFT  10
#define SCHED_FIXEDPOINT_SCALE  (1L << SCHED_FIXEDPOINT_SHIFT)

#define NICE_0_LOAD_SHIFT \
    (SCHED_FIXEDPOINT_SHIFT + SCHED_FIXEDPOINT_SHIFT)

/*
 * Task weight (visible to users) and its load (invisible to users) have
 * independent resolution, but they should be well calibrated. We use
 * scale_load() and scale_load_down(w) to convert between them. The
 * following must be true:
 *
 *  scale_load(sched_prio_to_weight[USER_PRIO(NICE_TO_PRIO(0))]) == NICE_0_LOAD
 *
 */
#define NICE_0_LOAD     (1L << NICE_0_LOAD_SHIFT)

#define ROOT_TASK_GROUP_LOAD    NICE_0_LOAD

#define RETRY_TASK      ((void *)-1UL)

struct task_struct;

extern unsigned long init_stack[THREAD_SIZE / sizeof(unsigned long)];

/* CPU-specific state of a task */
struct __riscv_d_ext_state {
    u64 f[32];
    u32 fcsr;
};

struct thread_struct {
    /* Callee-saved registers */
    unsigned long ra;
    unsigned long sp;   /* Kernel mode stack */
    unsigned long s[12];    /* s[0]: frame pointer */
    struct __riscv_d_ext_state fstate;
};

/* CFS-related fields in a runqueue */
struct cfs_rq {
    struct rb_root_cached tasks_timeline;

    /*
     * 'curr' points to currently running entity on this cfs_rq.
     * It is set to NULL otherwise (i.e when none are currently running).
     */
    struct sched_entity *curr;

    struct rq *rq;  /* CPU runqueue to which this cfs_rq is attached */
    struct task_group *tg;  /* group that "owns" this runqueue */
};

struct rq {
    struct cfs_rq   cfs;

    struct task_struct *curr;
    struct task_struct *idle;
};

struct sched_class {
    void (*enqueue_task)(struct rq *rq, struct task_struct *p, int flags);
};

struct sched_entity {
    struct rb_node run_node;
    struct list_head group_node;
    struct sched_entity *parent;
    struct cfs_rq       *cfs_rq;

    /* rq "owned" by this entity/group: */
    struct cfs_rq *my_q;

    unsigned int on_rq;
    u64 vruntime;
};

struct task_struct {
    struct thread_info thread_info;

    /* -1 unrunnable, 0 runnable, >0 stopped: */
    volatile long state;

    void *stack;

    /* Per task flags (PF_*), defined further below: */
    unsigned int flags;

    struct mm_struct *mm;
    struct mm_struct *active_mm;

    struct fs_struct *fs;

    /* Open file information: */
    struct files_struct *files;

    /* Namespaces: */
    struct nsproxy *nsproxy;

    /* Signal handlers: */
    struct signal_struct *signal;

    /* CPU-specific state of this task: */
    struct thread_struct thread;

    const struct sched_class *sched_class;
    struct sched_entity se;

    int prio;
    int normal_prio;

    int on_rq;
    int on_cpu;

    /* PID/PID hash table linkage. */
    struct pid *thread_pid;

    struct task_group *sched_task_group;
};

/* Task group related information */
struct task_group {
    /* schedulable entities of this group on each CPU */
    struct sched_entity **se;
    /* runqueue "owned" by this group on each CPU */
    struct cfs_rq       **cfs_rq;
    unsigned long       shares;
};

static struct rq runqueue;

typedef void (*schedule_tail_t)(struct task_struct *);
extern schedule_tail_t schedule_tail_func;

void wake_up_new_task(struct task_struct *p);

int sched_fork(unsigned long clone_flags, struct task_struct *p);

extern const struct sched_class fair_sched_class;

void init_cfs_rq(struct cfs_rq *cfs_rq);

static inline struct task_group *task_group(struct task_struct *p)
{
    return p->sched_task_group;
}

static inline void set_task_rq(struct task_struct *p, unsigned int cpu)
{
    struct task_group *tg = task_group(p);

    //set_task_rq_fair(&p->se, p->se.cfs_rq, tg->cfs_rq[cpu]);
    p->se.cfs_rq = tg->cfs_rq[cpu];
    p->se.parent = tg->se[cpu];
}

static inline void
__set_task_cpu(struct task_struct *p, unsigned int cpu)
{
    set_task_rq(p, cpu);
}

void init_tg_cfs_entry(struct task_group *tg, struct cfs_rq *cfs_rq,
                       struct sched_entity *se, int cpu,
                       struct sched_entity *parent);

void schedule_preempt_disabled(void);

struct task_struct *
pick_next_task_fair(struct rq *rq, struct task_struct *prev);

/* An entity is a task if it doesn't "own" a runqueue */
#define entity_is_task(se)  (!se->my_q)

/*
 * the helpers to get the task's different pids as they are seen
 * from various namespaces
 *
 * task_xid_nr()     : global id, i.e. the id seen from the init namespace;
 * task_xid_vnr()    : virtual id, i.e. the id seen from the pid namespace of
 *                     current.
 * task_xid_nr_ns()  : id seen from the ns specified;
 *
 * see also pid_nr() etc in include/linux/pid.h
 */
pid_t __task_pid_nr_ns(struct task_struct *task,
                       enum pid_type type,
                       struct pid_namespace *ns);

static inline pid_t task_tgid_vnr(struct task_struct *tsk)
{
    return __task_pid_nr_ns(tsk, PIDTYPE_TGID, NULL);
}

static inline struct pid *task_pid(struct task_struct *task)
{
    return task->thread_pid;
}

#endif /* _LINUX_SCHED_H */

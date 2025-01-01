// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <sched.h>
#include <export.h>
#include <cpumask.h>
#include <sched/rt.h>
#include <asm-switch_to.h>
#include <sched/deadline.h>

extern struct task_group root_task_group;

/* Cacheline aligned slab cache for task_group */
static struct kmem_cache *task_group_cache;

/**
 * schedule_tail - first thing a freshly forked thread must call.
 * @prev: the thread we just switched away from.
 */
void _schedule_tail(struct task_struct *prev)
{
    /* Todo: add it. */
}

struct rq *
__task_rq_lock(struct task_struct *p)
{
    return task_rq(p);
}

static inline void
enqueue_task(struct rq *rq, struct task_struct *p, int flags)
{
    p->sched_class->enqueue_task(rq, p, flags);
}

void activate_task(struct rq *rq, struct task_struct *p, int flags)
{
    enqueue_task(rq, p, flags);

    p->on_rq = TASK_ON_RQ_QUEUED;
}

/*
 * wake_up_new_task - wake up a newly created task for the first time.
 *
 * This function will do some initial scheduler statistics housekeeping
 * that must be done for every newly created context, then puts the task
 * on the runqueue and wakes it.
 */
void wake_up_new_task(struct task_struct *p)
{
    struct rq *rq;

    p->state = TASK_RUNNING;

    rq = __task_rq_lock(p);

    activate_task(rq, p, ENQUEUE_NOCLOCK);
}
EXPORT_SYMBOL(wake_up_new_task);

static void __sched_fork(unsigned long clone_flags, struct task_struct *p)
{
    p->on_rq = 0;

    p->se.on_rq = 0;
    p->se.vruntime = 0;
    INIT_LIST_HEAD(&p->se.group_node);

    p->se.cfs_rq = NULL;
}

int sched_fork(unsigned long clone_flags, struct task_struct *p)
{
    __sched_fork(clone_flags, p);

    p->state = TASK_NEW;

    /*
     * Make sure we do not leak PI boosting priority to the child.
     */
    p->prio = current->normal_prio;

    if (dl_prio(p->prio))
        panic("bad prio %d", p->prio);
    else if (rt_prio(p->prio))
        panic("no rt sched class!");
    else
        p->sched_class = &fair_sched_class;

    __set_task_cpu(p, 0);

    p->on_cpu = 0;
    return 0;
}
EXPORT_SYMBOL(sched_fork);

int alloc_fair_sched_group(struct task_group *tg)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se;

    tg->cfs_rq = kcalloc(nr_cpu_ids, sizeof(cfs_rq), GFP_KERNEL);
    if (!tg->cfs_rq)
        panic("out of memory!");

    tg->se = kcalloc(nr_cpu_ids, sizeof(se), GFP_KERNEL);
    if (!tg->se)
        panic("out of memory!");

    tg->shares = NICE_0_LOAD;

    cfs_rq = kzalloc_node(sizeof(struct cfs_rq), GFP_KERNEL);
    if (!cfs_rq)
        panic("out of memory!");

    se = kzalloc_node(sizeof(struct sched_entity), GFP_KERNEL);
    if (!se)
        panic("out of memory!");

    init_cfs_rq(cfs_rq);
    init_tg_cfs_entry(tg, cfs_rq, se, 0, NULL);
    return 1;
}

/* allocate runqueue etc for a new task group */
static struct task_group *
sched_create_group(void)
{
    struct task_group *tg;

    tg = kmem_cache_alloc(task_group_cache, GFP_KERNEL | __GFP_ZERO);
    if (!tg)
        panic("out of memory!");

    if (!alloc_fair_sched_group(tg))
        panic("alloc fair sched group error!");

    return tg;
}

static inline struct task_struct *
pick_next_task(struct rq *rq, struct task_struct *prev)
{
    struct task_struct *p;
    const struct sched_class *class;

    /*
     * Optimization: we know that if all tasks are in the fair class we can
     * call that function directly, but only if the @prev task wasn't of a
     * higher scheduling class, because otherwise those loose the
     * opportunity to pull in more work from other CPUs.
     */
    if (likely(prev->sched_class <= &fair_sched_class)) {

        p = pick_next_task_fair(rq, prev);
        if (unlikely(p == RETRY_TASK))
            panic("need to retry task!");

        return p;
    }

    panic("%s: !", __func__);
}

static inline void prepare_task(struct task_struct *next)
{
    /*
     * Claim the task as running, we do this before switching to it
     * such that any running task will have this set.
     *
     * See the ttwu() WF_ON_CPU case and its ordering comment.
     */
    next->on_cpu = 1;
}

static inline void
prepare_task_switch(struct rq *rq,
                    struct task_struct *prev,
                    struct task_struct *next)
{
    prepare_task(next);
}

static struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
               struct task_struct *next)
{
    prepare_task_switch(rq, prev, next);

    printk("switch ...\n");

    /* Here we just switch the register state and the stack. */
    switch_to(prev, next, prev);

    panic("Never reach here!");
}

static void __schedule(bool preempt)
{
    struct rq *rq;
    struct task_struct *prev, *next;

    rq = cpu_rq();
    prev = rq->curr;

    next = pick_next_task(rq, prev);

    if (likely(prev != next)) {
        /*
         * RCU users of rcu_dereference(rq->curr) may not see
         * changes to task_struct made by pick_next_task().
         */
        rq->curr = next;

        /* Also unlocks the rq: */
        rq = context_switch(rq, prev, next);
    } else {
        panic("still IS idle task!");
    }

    panic("%s: !", __func__);
}

void schedule(void)
{
    __schedule(false);
}
EXPORT_SYMBOL(schedule);

/**
 * schedule_preempt_disabled - called with preemption disabled
 *
 * Returns with preemption disabled. Note: preempt_count must be 1
 */
void schedule_preempt_disabled(void)
{
    schedule();
}
EXPORT_SYMBOL(schedule_preempt_disabled);

void init_idle(struct task_struct *idle, int cpu)
{
    struct rq *rq = cpu_rq();

    __sched_fork(0, idle);

    idle->state = TASK_RUNNING;
    idle->flags |= PF_IDLE;

    rq->idle = idle;
    rq->curr = idle;
    idle->on_rq = TASK_ON_RQ_QUEUED;
    idle->on_cpu = 1;
}

void sched_init(void)
{
    struct rq *rq;
    unsigned long ptr = 0;

    ptr += 2 * nr_cpu_ids * sizeof(void **);

    ptr = (unsigned long)kzalloc(ptr, GFP_NOWAIT);

    root_task_group.se = (struct sched_entity **)ptr;
    ptr += nr_cpu_ids * sizeof(void **);

    root_task_group.cfs_rq = (struct cfs_rq **)ptr;
    ptr += nr_cpu_ids * sizeof(void **);

    root_task_group.shares = ROOT_TASK_GROUP_LOAD;

    rq = cpu_rq();
    init_cfs_rq(&rq->cfs);
    init_tg_cfs_entry(&root_task_group, &rq->cfs, NULL, 0, NULL);

    task_group_cache = KMEM_CACHE(task_group, 0);

    sched_create_group();

    /*
     * Make us the idle thread. Technically, schedule() should not be
     * called from this thread, however somewhere below it might be,
     * but because we are the idle thread, we just pick up running again
     * when this runqueue becomes "idle".
     */
    init_idle(current, 0);
}

int
init_module(void)
{
    printk("module[sched]: init begin ...\n");

    schedule_tail_func = _schedule_tail;

    sched_init();

    printk("module[sched]: init end!\n");

    return 0;
}

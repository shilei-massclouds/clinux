// SPDX-License-Identifier: GPL-2.0

#include <sched.h>

/* Walk up scheduling entities hierarchy */
#define for_each_sched_entity(se) for(; se; se = se->parent)

/* runqueue on which this entity is (to be) queued */
static inline struct cfs_rq *cfs_rq_of(struct sched_entity *se)
{
    return se->cfs_rq;
}

static inline int
entity_before(struct sched_entity *a, struct sched_entity *b)
{
    return (s64)(a->vruntime - b->vruntime) < 0;
}

/*
 * Enqueue an entity into the rb-tree:
 */
static void __enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    struct sched_entity *entry;
    bool leftmost = true;
    struct rb_node *parent = NULL;
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_root.rb_node;

    /*
     * Find the right place in the rbtree:
     */
    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct sched_entity, run_node);
        /*
         * We dont care about collisions. Nodes with
         * the same key stay together.
         */
        if (entity_before(se, entry)) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = false;
        }
    }

    rb_link_node(&se->run_node, parent, link);
    rb_insert_color_cached(&se->run_node, &cfs_rq->tasks_timeline, leftmost);
}

static void
enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    bool curr = cfs_rq->curr == se;

    if (!curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;
}

void init_cfs_rq(struct cfs_rq *cfs_rq)
{
    cfs_rq->tasks_timeline = RB_ROOT_CACHED;
}

void init_tg_cfs_entry(struct task_group *tg, struct cfs_rq *cfs_rq,
                       struct sched_entity *se, int cpu,
                       struct sched_entity *parent)
{
    struct rq *rq = cpu_rq();

    cfs_rq->tg = tg;
    cfs_rq->rq = rq;

    tg->cfs_rq[cpu] = cfs_rq;
    tg->se[cpu] = se;

    /* se could be NULL for root_task_group */
    if (!se)
        return;

    if (!parent) {
        se->cfs_rq = &rq->cfs;
    } else {
        se->cfs_rq = parent->my_q;
    }

    se->my_q = cfs_rq;
    se->parent = parent;
}

struct sched_entity *__pick_first_entity(struct cfs_rq *cfs_rq)
{
    struct rb_node *left = rb_first_cached(&cfs_rq->tasks_timeline);

    if (!left)
        return NULL;

    return rb_entry(left, struct sched_entity, run_node);
}

static struct sched_entity *
pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    struct sched_entity *se;
    struct sched_entity *left = __pick_first_entity(cfs_rq);

    /*
     * If curr is set we have to see if its left of the leftmost entity
     * still in the tree, provided there was anything in the tree at all.
     */
    if (!left || (curr && entity_before(curr, left)))
        left = curr;

    se = left; /* ideally we run the leftmost entity */
    return se;
}

static inline struct task_struct *task_of(struct sched_entity *se)
{
    BUG_ON(!entity_is_task(se));
    return container_of(se, struct task_struct, se);
}

static void
__dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    rb_erase_cached(&se->run_node, &cfs_rq->tasks_timeline);
}

static void
set_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    /* 'current' is not kept within the tree. */
    if (se->on_rq) {
        /*
         * Any task has to be enqueued before it get to execute on
         * a CPU. So account for the time it spent waiting on the
         * runqueue.
         */
        __dequeue_entity(cfs_rq, se);
    }

    cfs_rq->curr = se;
}

/* runqueue "owned" by this group */
static inline struct cfs_rq *group_cfs_rq(struct sched_entity *grp)
{
    return grp->my_q;
}

struct task_struct *
pick_next_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct task_struct *p;
    struct sched_entity *se;
    struct cfs_rq *cfs_rq = &rq->cfs;

    do {
        se = pick_next_entity(cfs_rq, NULL);
        set_next_entity(cfs_rq, se);
        cfs_rq = group_cfs_rq(se);
        printk("%s: 1\n", __func__);
    } while (cfs_rq);

    p = task_of(se);
    return p;
}

/*
 * The enqueue_task method is called before nr_running is
 * increased. Here we update the fair scheduling stats and
 * then put the task into the rbtree:
 */
static void
enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se) {
        if (se->on_rq)
            break;

        cfs_rq = cfs_rq_of(se);
        printk("%s: cfs_rq(%lx)\n", __func__, cfs_rq);
        enqueue_entity(cfs_rq, se, flags);

        flags = ENQUEUE_WAKEUP;
    }
}

/*
 * All the scheduling class methods:
 */
const struct sched_class fair_sched_class = {
    .enqueue_task = enqueue_task_fair,
};

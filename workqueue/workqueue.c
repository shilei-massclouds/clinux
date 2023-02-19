// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <errno.h>
#include <export.h>
#include <printk.h>
#include <string.h>
#include <jiffies.h>
#include <workqueue.h>

#define MIN_NICE    -20

#define for_each_cpu_worker_pool(pool) \
    for ((pool) = &cpu_worker_pools[0];   \
         (pool) < &cpu_worker_pools[NR_STD_WORKER_POOLS]; \
         (pool)++)

enum {
    /*
     * worker_pool flags
     *
     * A bound pool is either associated or disassociated with its CPU.
     * While associated (!DISASSOCIATED), all workers are bound to the
     * CPU and none has %WORKER_UNBOUND set and concurrency management
     * is in effect.
     *
     * While DISASSOCIATED, the cpu may be offline and all workers have
     * %WORKER_UNBOUND set and concurrency management disabled, and may
     * be executing on any CPU.  The pool behaves as an unbound one.
     *
     * Note that DISASSOCIATED should be flipped only while holding
     * wq_pool_attach_mutex to avoid changing binding state while
     * worker_attach_to_pool() is in progress.
     */
    POOL_MANAGER_ACTIVE = 1 << 0,   /* being managed */
    POOL_DISASSOCIATED  = 1 << 2,   /* cpu can't serve workers */

    /* worker flags */
    WORKER_DIE      = 1 << 1,   /* die die die */
    WORKER_IDLE     = 1 << 2,   /* is idle */
    WORKER_PREP     = 1 << 3,   /* preparing to run works */
    WORKER_CPU_INTENSIVE    = 1 << 6,   /* cpu intensive */
    WORKER_UNBOUND      = 1 << 7,   /* worker is unbound */
    WORKER_REBOUND      = 1 << 8,   /* worker was rebound */

    WORKER_NOT_RUNNING  = WORKER_PREP | WORKER_CPU_INTENSIVE |
                  WORKER_UNBOUND | WORKER_REBOUND,

    NR_STD_WORKER_POOLS = 2,        /* # standard pools per cpu */

    UNBOUND_POOL_HASH_ORDER = 6,        /* hashed by pool->attrs */
    BUSY_WORKER_HASH_ORDER  = 6,        /* 64 pointers */

    MAX_IDLE_WORKERS_RATIO  = 4,        /* 1/4 of busy can be idle */
    IDLE_WORKER_TIMEOUT = 300 * HZ, /* keep idle ones for 5 mins */

    MAYDAY_INITIAL_TIMEOUT  = HZ / 100 >= 2 ? HZ / 100 : 2,
                        /* call for help after 10ms
                           (min two ticks) */
    MAYDAY_INTERVAL     = HZ / 10,  /* and then every 100ms */
    CREATE_COOLDOWN     = HZ,       /* time to breath after fail */

    /*
     * Rescue workers are used only on emergencies and shared by
     * all cpus.  Give MIN_NICE.
     */
    RESCUER_NICE_LEVEL  = MIN_NICE,
    HIGHPRI_NICE_LEVEL  = MIN_NICE,

    WQ_NAME_LEN     = 24,
};

struct workqueue_struct {
    unsigned int flags;                 /* WQ: WQ_* flags */
    struct pool_workqueue *cpu_pwqs;    /* I: per-cpu pwqs */
};

static struct worker_pool cpu_worker_pools[NR_STD_WORKER_POOLS];

static void
init_pwq(struct pool_workqueue *pwq,
         struct workqueue_struct *wq,
         struct worker_pool *pool)
{
    memset(pwq, 0, sizeof(*pwq));

    pwq->pool = pool;
    pwq->wq = wq;
    /*
    pwq->flush_color = -1;
    pwq->refcnt = 1;
    INIT_LIST_HEAD(&pwq->delayed_works);
    INIT_LIST_HEAD(&pwq->pwqs_node);
    INIT_LIST_HEAD(&pwq->mayday_node);
    INIT_WORK(&pwq->unbound_release_work, pwq_unbound_release_workfn);
    */
}

static void link_pwq(struct pool_workqueue *pwq)
{
    /*
    struct workqueue_struct *wq = pwq->wq;

    if (!list_empty(&pwq->pwqs_node))
        return;

    pwq->work_color = wq->work_color;

    pwq_adjust_max_active(pwq);

    list_add_rcu(&pwq->pwqs_node, &wq->pwqs);
    */
}

static int alloc_and_link_pwqs(struct workqueue_struct *wq)
{
    bool highpri = wq->flags & WQ_HIGHPRI;

    if (!(wq->flags & WQ_UNBOUND)) {
        struct pool_workqueue *pwq;
        struct worker_pool *cpu_pools;

        wq->cpu_pwqs = kzalloc(sizeof(struct pool_workqueue), GFP_KERNEL);
        if (!wq->cpu_pwqs)
            return -ENOMEM;

        printk("%s: cpu_pwqs(%p)\n", __func__, wq->cpu_pwqs);
        pwq = wq->cpu_pwqs;
        cpu_pools = cpu_worker_pools;

        init_pwq(pwq, wq, &cpu_pools[highpri]);
        link_pwq(pwq);
        return 0;
    }

    panic("bad flags!");
}

struct workqueue_struct *
alloc_workqueue(const char *fmt, unsigned int flags, int max_active)
{
    struct workqueue_struct *wq;
    size_t tbl_size = 0;

    wq = kzalloc(sizeof(*wq) + tbl_size, GFP_KERNEL);
    if (!wq)
        return NULL;

    if (alloc_and_link_pwqs(wq) < 0)
        panic("bad pwqs!");

    return wq;
}
EXPORT_SYMBOL(alloc_workqueue);

static int
try_to_grab_pending(struct work_struct *work, bool is_dwork,
                    unsigned long *flags)
{
    BUG_ON(!is_dwork);

    return 0;
}

static inline void
set_work_data(struct work_struct *work,
              unsigned long data,
              unsigned long flags)
{
    atomic_long_set(&work->data, data | flags);
}

static void
set_work_pwq(struct work_struct *work,
             struct pool_workqueue *pwq,
             unsigned long extra_flags)
{
    set_work_data(work, (unsigned long)pwq,
                  WORK_STRUCT_PENDING|WORK_STRUCT_PWQ|extra_flags);
}

static void process_one_work(struct work_struct *work)
{
    list_del_init(&work->entry);

    work->func(work);
}

/* Do I need to keep working?  Called from currently running workers. */
static bool keep_working(struct worker_pool *pool)
{
    return !list_empty(&pool->worklist);
}

static void
insert_work(struct pool_workqueue *pwq, struct work_struct *work,
            struct list_head *head, unsigned int extra_flags)
{
    struct worker_pool *pool = pwq->pool;

    /* we own @work, set data and link */
    set_work_pwq(work, pwq, extra_flags);
    list_add_tail(&work->entry, head);

    /* !!! NOTICE !!! */
    /* avoid starting another process */
    do {
        struct work_struct *work;

        work = list_first_entry(&pool->worklist,
                                struct work_struct, entry);

        /* optimization path, not strictly necessary */
        process_one_work(work);
    } while (keep_working(pool));
}

static void
__queue_work(int cpu, struct workqueue_struct *wq, struct work_struct *work)
{
    struct pool_workqueue *pwq;
    struct list_head *worklist;

    BUG_ON(wq->flags & WQ_UNBOUND);

    pwq = wq->cpu_pwqs;

    worklist = &pwq->pool->worklist;

    insert_work(pwq, work, worklist, 0);
}

static void
__queue_delayed_work(int cpu, struct workqueue_struct *wq,
                     struct delayed_work *dwork, unsigned long delay)
{
    if (!delay) {
        __queue_work(cpu, wq, &dwork->work);
        return;
    }

    panic("%s: !", __func__);
}

bool
mod_delayed_work_on(int cpu,
                    struct workqueue_struct *wq,
                    struct delayed_work *dwork,
                    unsigned long delay)
{
    int ret;
    unsigned long flags;

    do {
        ret = try_to_grab_pending(&dwork->work, true, &flags);
    } while (unlikely(ret == -EAGAIN));

    if (likely(ret >= 0)) {
        __queue_delayed_work(cpu, wq, dwork, delay);
    }

    /* -ENOENT from try_to_grab_pending() becomes %true */
    return ret;
}
EXPORT_SYMBOL(mod_delayed_work_on);

static int init_worker_pool(struct worker_pool *pool)
{
    INIT_LIST_HEAD(&pool->worklist);

    return 0;
}

void workqueue_init(void)
{
    struct worker_pool *pool;

    for_each_cpu_worker_pool(pool)
        init_worker_pool(pool);
}

int
init_module(void)
{
    printk("module[workqueue]: init begin ...\n");

    workqueue_init();

    printk("module[workqueue]: init end!\n");
    return 0;
}

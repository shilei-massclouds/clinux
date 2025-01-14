/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_WORKQUEUE_H
#define _LINUX_WORKQUEUE_H

#include <atomic.h>

/*
 * The first word is the work queue pointer and
 * the flags rolled into one
 */
#define work_data_bits(work) ((unsigned long *)(&(work)->data))

#define __INIT_WORK(_work, _func, _onstack) \
    do {                                    \
        INIT_LIST_HEAD(&(_work)->entry);    \
        (_work)->func = (_func);            \
    } while (0)

#define INIT_WORK(_work, _func) \
    __INIT_WORK((_work), (_func), 0)

#define __INIT_DELAYED_WORK(_work, _func, _tflags)  \
    do {                                            \
        INIT_WORK(&(_work)->work, (_func));         \
    } while (0)

#define INIT_DELAYED_WORK(_work, _func) \
    __INIT_DELAYED_WORK(_work, _func, 0)

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

enum {
    WORK_STRUCT_PENDING_BIT = 0,    /* work item is pending execution */
    WORK_STRUCT_DELAYED_BIT = 1,    /* work item is delayed */
    WORK_STRUCT_PWQ_BIT = 2,        /* data points to pwq */
    WORK_STRUCT_LINKED_BIT  = 3,    /* next work is linked to this one */
    WORK_STRUCT_COLOR_SHIFT = 4,    /* color for workqueue flushing */
    WORK_STRUCT_COLOR_BITS  = 4,

    WORK_STRUCT_PENDING = 1 << WORK_STRUCT_PENDING_BIT,
    WORK_STRUCT_DELAYED = 1 << WORK_STRUCT_DELAYED_BIT,
    WORK_STRUCT_PWQ     = 1 << WORK_STRUCT_PWQ_BIT,
    WORK_STRUCT_LINKED  = 1 << WORK_STRUCT_LINKED_BIT,
    WORK_STRUCT_STATIC  = 0,
    /*
     * The last color is no color used for works which don't
     * participate in workqueue flushing.
     */
    WORK_NR_COLORS      = (1 << WORK_STRUCT_COLOR_BITS) - 1,
    WORK_NO_COLOR       = WORK_NR_COLORS,

    /* not bound to any CPU, prefer the local CPU */
    WORK_CPU_UNBOUND    = NR_CPUS,

    /*
     * Reserve 8 bits off of pwq pointer w/ debugobjects turned off.
     * This makes pwqs aligned to 256 bytes and allows 15 workqueue
     * flush colors.
     */
    WORK_STRUCT_FLAG_BITS   = WORK_STRUCT_COLOR_SHIFT +
                  WORK_STRUCT_COLOR_BITS,

    /* data contains off-queue information when !WORK_STRUCT_PWQ */
    WORK_OFFQ_FLAG_BASE = WORK_STRUCT_COLOR_SHIFT,

    __WORK_OFFQ_CANCELING   = WORK_OFFQ_FLAG_BASE,
    WORK_OFFQ_CANCELING = (1 << __WORK_OFFQ_CANCELING),

    /*
     * When a work item is off queue, its high bits point to the last
     * pool it was on.  Cap at 31 bits and use the highest number to
     * indicate that no pool is associated.
     */
    WORK_OFFQ_FLAG_BITS = 1,
    WORK_OFFQ_POOL_SHIFT    = WORK_OFFQ_FLAG_BASE + WORK_OFFQ_FLAG_BITS,
    WORK_OFFQ_LEFT      = BITS_PER_LONG - WORK_OFFQ_POOL_SHIFT,
    WORK_OFFQ_POOL_BITS = WORK_OFFQ_LEFT <= 31 ? WORK_OFFQ_LEFT : 31,
    WORK_OFFQ_POOL_NONE = (1LU << WORK_OFFQ_POOL_BITS) - 1,
    /* convenience constants */
    WORK_STRUCT_FLAG_MASK   = (1UL << WORK_STRUCT_FLAG_BITS) - 1,
    WORK_STRUCT_WQ_DATA_MASK = ~WORK_STRUCT_FLAG_MASK,
    WORK_STRUCT_NO_POOL = (unsigned long)WORK_OFFQ_POOL_NONE << WORK_OFFQ_POOL_SHIFT,

    /* bit mask for work_busy() return values */
    WORK_BUSY_PENDING   = 1 << 0,
    WORK_BUSY_RUNNING   = 1 << 1,

    /* maximum string length for set_worker_desc() */
    WORKER_DESC_LEN     = 24,
};

/*
 * Workqueue flags and constants.  For details, please refer to
 * Documentation/core-api/workqueue.rst.
 */
enum {
    WQ_UNBOUND          = 1 << 1, /* not bound to any cpu */
    WQ_FREEZABLE        = 1 << 2, /* freeze during suspend */
    WQ_MEM_RECLAIM      = 1 << 3, /* may be used for memory reclaim */
    WQ_HIGHPRI          = 1 << 4, /* high priority */
    WQ_CPU_INTENSIVE    = 1 << 5, /* cpu intensive workqueue */
    WQ_SYSFS            = 1 << 6, /* visible in sysfs, see wq_sysfs_register() */
    WQ_POWER_EFFICIENT  = 1 << 7,

    __WQ_DRAINING       = 1 << 16, /* internal: workqueue is draining */
    __WQ_ORDERED        = 1 << 17, /* internal: workqueue is ordered */
    __WQ_LEGACY         = 1 << 18, /* internal: create*_workqueue() */
    __WQ_ORDERED_EXPLICIT   = 1 << 19, /* internal: alloc_ordered_workqueue() */

    WQ_MAX_ACTIVE       = 512,    /* I like 512, better ideas? */
    WQ_MAX_UNBOUND_PER_CPU  = 4,      /* 4 * #cpus for unbound wq */
    WQ_DFL_ACTIVE       = WQ_MAX_ACTIVE / 2,
};

struct work_struct {
    atomic_long_t data;
    struct list_head entry;
    work_func_t func;
};

struct delayed_work {
    struct work_struct work;
};

struct pool_workqueue {
    struct worker_pool *pool;       /* I: the associated pool */
    struct workqueue_struct *wq;    /* I: the owning workqueue */
};

struct worker_pool {
    struct list_head worklist;      /* L: list of pending works */
};

struct workqueue_struct *
alloc_workqueue(const char *fmt, unsigned int flags, int max_active);

bool
mod_delayed_work_on(int cpu, struct workqueue_struct *wq,
                    struct delayed_work *dwork, unsigned long delay);

#endif /* _LINUX_WORKQUEUE_H */

// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <list.h>
#include <slab.h>
#include <errno.h>
#include <blkdev.h>
#include <export.h>
#include <elevator.h>
#include <blk-mq-sched.h>

static LIST_HEAD(elv_list);

static bool
elevator_match(const struct elevator_type *e, const char *name)
{
    if (!strcmp(e->elevator_name, name))
        return true;

    return false;
}

static struct elevator_type *
elevator_find(const char *name)
{
    struct elevator_type *e;

    list_for_each_entry(e, &elv_list, list) {
        if (elevator_match(e, name))
            return e;
    }

    return NULL;
}

static struct elevator_type *
elevator_get(struct request_queue *q, const char *name)
{
    return elevator_find(name);
}

static struct elevator_type *
elevator_get_default(struct request_queue *q)
{
    if (q->nr_hw_queues != 1)
        return NULL;

    return elevator_get(q, "mq-deadline");
}

void elevator_init_mq(struct request_queue *q)
{
    int err;
    struct elevator_type *e;

    e = elevator_get_default(q);
    if (!e)
        panic("bad elevator!");

    err = blk_mq_init_sched(q, e);
    if (err)
        panic("can not init sched!");
}
EXPORT_SYMBOL(elevator_init_mq);

struct elevator_queue *
elevator_alloc(struct request_queue *q, struct elevator_type *e)
{
    struct elevator_queue *eq;

    eq = kzalloc(sizeof(*eq), GFP_KERNEL);
    if (unlikely(!eq))
        return NULL;

    eq->type = e;
    /*
    kobject_init(&eq->kobj, &elv_ktype);
    hash_init(eq->hash);
    */

    return eq;
}
EXPORT_SYMBOL(elevator_alloc);

int elv_register(struct elevator_type *e)
{
    if (elevator_find(e->elevator_name))
        return -EBUSY;

    list_add_tail(&e->list, &elv_list);
    printk("io scheduler %s registered\n", e->elevator_name);
    return 0;
}

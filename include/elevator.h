/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_ELEVATOR_H
#define _LINUX_ELEVATOR_H

#include <blkdev.h>

#define rq_entry_fifo(ptr)  list_entry((ptr), struct request, queuelist)

struct elevator_type;

struct elevator_mq_ops {
    void (*insert_requests)(struct blk_mq_hw_ctx *, struct list_head *, bool);
    struct request *(*dispatch_request)(struct blk_mq_hw_ctx *);
    bool (*has_work)(struct blk_mq_hw_ctx *);
    int (*init_sched)(struct request_queue *, struct elevator_type *);
};

struct elevator_type {
    /* fields provided by elevator implementation */
    struct elevator_mq_ops ops;

    const char *elevator_name;
    struct list_head list;
};

/*
 * each queue has an elevator_queue associated with it
 */
struct elevator_queue
{
    struct elevator_type *type;
    void *elevator_data;
    struct kobject kobj;
    /*
    unsigned int registered:1;
    DECLARE_HASHTABLE(hash, ELV_HASH_BITS);
    */
};

int elv_register(struct elevator_type *e);

void elevator_init_mq(struct request_queue *q);

struct elevator_queue *
elevator_alloc(struct request_queue *q, struct elevator_type *e);

#endif /* _LINUX_ELEVATOR_H */

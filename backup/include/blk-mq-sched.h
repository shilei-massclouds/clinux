/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BLK_MQ_INIT_SCHED_H
#define _BLK_MQ_INIT_SCHED_H

#include <blk-mq.h>
#include <elevator.h>

int blk_mq_init_sched(struct request_queue *q, struct elevator_type *e);

static inline bool blk_mq_sched_has_work(struct blk_mq_hw_ctx *hctx)
{
    struct elevator_queue *e = hctx->queue->elevator;

    if (e && e->type->ops.has_work)
        return e->type->ops.has_work(hctx);

    return false;
}

#endif /* _BLK_MQ_INIT_SCHED_H */

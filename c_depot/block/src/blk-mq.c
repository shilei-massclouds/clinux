// SPDX-License-Identifier: GPL-2.0+

#include <blk.h>
#include <bug.h>
#include <slab.h>
#include <errno.h>
#include <blk-mq.h>
#include <export.h>
#include <kernel.h>
#include <printk.h>
#include <hardirq.h>
#include <jiffies.h>
#include <elevator.h>
#include <interrupt.h>
#include <blk-mq-sched.h>

static struct list_head blk_cpu_done;

static int blk_mq_hw_ctx_size(struct blk_mq_tag_set *tag_set)
{
    int hw_ctx_size = sizeof(struct blk_mq_hw_ctx);
    return hw_ctx_size;
}

bool
blk_mq_dispatch_rq_list(struct blk_mq_hw_ctx *hctx,
                        struct list_head *list,
                        unsigned int nr_budgets)
{
    struct request *rq;
    int errors, queued;
    blk_status_t ret = BLK_STS_OK;
    struct request_queue *q = hctx->queue;

    if (list_empty(list))
        return false;

    do {
        struct blk_mq_queue_data bd;

        rq = list_first_entry(list, struct request, queuelist);

        list_del_init(&rq->queuelist);
        bd.rq = rq;
        bd.last = true;

        ret = q->mq_ops->queue_rq(hctx, &bd);
        switch (ret) {
        case BLK_STS_OK:
            queued++;
            break;
        case BLK_STS_RESOURCE:
        case BLK_STS_DEV_RESOURCE:
            panic("BLK_STS_DEV_RESOURCE!");
        case BLK_STS_ZONE_RESOURCE:
            panic("BLK_STS_ZONE_RESOURCE!");
        default:
            errors++;
            panic("default!");
        }

    } while (!list_empty(list));

    return (queued + errors) != 0;
}

static int __blk_mq_do_dispatch_sched(struct blk_mq_hw_ctx *hctx)
{
    int count = 0;
    bool dispatched = false;
    struct request_queue *q = hctx->queue;
    struct elevator_queue *e = q->elevator;
    LIST_HEAD(rq_list);
    unsigned int max_dispatch = hctx->queue->nr_requests;

    do {
        struct request *rq;

        if (e->type->ops.has_work && !e->type->ops.has_work(hctx))
            break;

        rq = e->type->ops.dispatch_request(hctx);
        if (!rq)
            panic("%s: bad request!", __func__);

        list_add_tail(&rq->queuelist, &rq_list);
    } while (++count < max_dispatch);

    dispatched = blk_mq_dispatch_rq_list(hctx, &rq_list, count);
    return !!dispatched;
}

static int blk_mq_do_dispatch_sched(struct blk_mq_hw_ctx *hctx)
{
    int ret;

    do {
        ret = __blk_mq_do_dispatch_sched(hctx);
    } while (ret == 1);

    return ret;
}

static int __blk_mq_sched_dispatch_requests(struct blk_mq_hw_ctx *hctx)
{
    return blk_mq_do_dispatch_sched(hctx);
}

void blk_mq_sched_dispatch_requests(struct blk_mq_hw_ctx *hctx)
{
    int ret;

    ret = __blk_mq_sched_dispatch_requests(hctx);
    BUG_ON(ret);
}

/**
 * __blk_mq_run_hw_queue - Run a hardware queue.
 * @hctx: Pointer to the hardware queue to run.
 *
 * Send pending requests to the hardware.
 */
static void __blk_mq_run_hw_queue(struct blk_mq_hw_ctx *hctx)
{
    blk_mq_sched_dispatch_requests(hctx);
}

static void blk_mq_run_work_fn(struct work_struct *work)
{
    struct blk_mq_hw_ctx *hctx;

    hctx = container_of(work, struct blk_mq_hw_ctx, run_work.work);

    /*
     * If we are stopped, don't run the queue.
     */
    if (test_bit(BLK_MQ_S_STOPPED, &hctx->state))
        return;

    __blk_mq_run_hw_queue(hctx);
}

static struct blk_mq_hw_ctx *
blk_mq_alloc_hctx(struct request_queue *q, struct blk_mq_tag_set *set)
{
    struct blk_mq_hw_ctx *hctx;
    gfp_t gfp = GFP_NOIO | __GFP_NOWARN | __GFP_NORETRY;

    hctx = kzalloc(blk_mq_hw_ctx_size(set), gfp);
    if (!hctx)
        panic("out of memory!");

    INIT_DELAYED_WORK(&hctx->run_work, blk_mq_run_work_fn);
    hctx->queue = q;
    return hctx;
}

static int
blk_mq_init_hctx(struct request_queue *q,
                 struct blk_mq_tag_set *set,
                 struct blk_mq_hw_ctx *hctx, unsigned hctx_idx)
{
    /* Todo */
}

static struct blk_mq_hw_ctx *
blk_mq_alloc_and_init_hctx(struct blk_mq_tag_set *set,
                           struct request_queue *q,
                           int hctx_idx)
{
    struct blk_mq_hw_ctx *hctx;

    hctx = blk_mq_alloc_hctx(q, set);
    if (!hctx)
        panic("out of memory!");

    if (blk_mq_init_hctx(q, set, hctx, hctx_idx))
        panic("can not init hctx!");

    return hctx;
}

static void
blk_mq_realloc_hw_ctxs(struct blk_mq_tag_set *set,
                       struct request_queue *q)
{
    int i;
    struct blk_mq_hw_ctx **hctxs;

    BUG_ON(q->nr_hw_queues);

    hctxs = kcalloc(set->nr_hw_queues, sizeof(*hctxs), GFP_KERNEL);
    if (!hctxs)
        panic("out of memory!");

    q->queue_hw_ctx = hctxs;
    q->nr_hw_queues = set->nr_hw_queues;

    for (i = 0; i < set->nr_hw_queues; i++) {
        struct blk_mq_hw_ctx *hctx;

        if (hctxs[i])
            continue;

        hctx = blk_mq_alloc_and_init_hctx(set, q, i);
        if (hctx)
            hctxs[i] = hctx;
        else
            panic("Allocate new hctx fails");
    }
}

static int blk_mq_alloc_ctxs(struct request_queue *q)
{
    struct blk_mq_ctxs *ctxs;

    ctxs = kzalloc(sizeof(*ctxs), GFP_KERNEL);
    if (!ctxs)
        return -ENOMEM;

    ctxs->queue_ctx = kzalloc(sizeof(struct blk_mq_ctx), GFP_KERNEL);
    if (!ctxs->queue_ctx)
        panic("out of memory!");

    ctxs->queue_ctx->ctxs = ctxs;

    //q->mq_kobj = &ctxs->kobj;
    q->queue_ctx = ctxs->queue_ctx;

    return 0;
}

static void blk_mq_map_swqueue(struct request_queue *q)
{
    int i;
    struct blk_mq_ctx *ctx = q->queue_ctx;
    for (i = 0; i < HCTX_TYPE_POLL; i++)
        ctx->hctxs[i] = q->queue_hw_ctx[0];
}

struct request_queue *
blk_mq_init_allocated_queue(struct blk_mq_tag_set *set,
                            struct request_queue *q,
                            bool elevator_init)
{
    /* mark the queue as mq asap */
    q->mq_ops = set->ops;

    if (blk_mq_alloc_ctxs(q))
        panic("bad ctxs!");

    blk_mq_realloc_hw_ctxs(set, q);
    if (!q->nr_hw_queues)
        panic("nr_hw_queues ZERO!");

    q->tag_set = set;
    blk_mq_map_swqueue(q);
    return q;
}

struct request_queue *
blk_mq_init_queue_data(struct blk_mq_tag_set *set, void *queuedata)
{
    struct request_queue *uninit_q, *q;

    uninit_q = blk_alloc_queue();
    if (!uninit_q)
        return ERR_PTR(-ENOMEM);
    uninit_q->queuedata = queuedata;

    /*
     * Initialize the queue without an elevator. device_add_disk() will do
     * the initialization.
     */
    q = blk_mq_init_allocated_queue(set, uninit_q, false);
    if (IS_ERR(q))
        panic("can not init queue!");

    return q;
}
EXPORT_SYMBOL(blk_mq_init_queue_data);

struct request_queue *
blk_mq_init_queue(struct blk_mq_tag_set *set)
{
    return blk_mq_init_queue_data(set, NULL);
}
EXPORT_SYMBOL(blk_mq_init_queue);

static struct request *
blk_mq_rq_ctx_init(struct blk_mq_alloc_data *data,
                   unsigned int tag,
                   u64 alloc_time_ns)
{
    struct blk_mq_tags *tags = blk_mq_tags_from_data(data);
    struct request *rq = tags->static_rqs[tag];

    rq->q = data->q;
    rq->mq_ctx = data->ctx;
    rq->mq_hctx = data->hctx;
    rq->rq_flags = 0;
    rq->cmd_flags = data->cmd_flags;
    rq->nr_phys_segments = 0;
    INIT_LIST_HEAD(&rq->queuelist);
    return rq;
}

static struct request *
__blk_mq_alloc_request(struct blk_mq_alloc_data *data)
{
    unsigned int tag;
    struct request_queue *q = data->q;

    data->ctx = blk_mq_get_ctx(q);
    data->hctx = blk_mq_map_queue(q, data->cmd_flags, data->ctx);

    /* Todo: */
    tag = 1;
    return blk_mq_rq_ctx_init(data, tag, 0);
}

static bool blk_mq_hctx_has_pending(struct blk_mq_hw_ctx *hctx)
{
    return blk_mq_sched_has_work(hctx);
}

static void
__blk_mq_delay_run_hw_queue(struct blk_mq_hw_ctx *hctx,
                            bool async,
                            unsigned long msecs)
{
    kblockd_mod_delayed_work_on(0, &hctx->run_work, msecs_to_jiffies(msecs));
}

void blk_mq_run_hw_queue(struct blk_mq_hw_ctx *hctx, bool async)
{
    bool need_run;

    need_run = blk_mq_hctx_has_pending(hctx);

    if (need_run)
        __blk_mq_delay_run_hw_queue(hctx, async, 0);
}
EXPORT_SYMBOL(blk_mq_run_hw_queue);

void
blk_mq_sched_insert_request(struct request *rq,
                            bool at_head,
                            bool run_queue,
                            bool async)
{
    struct request_queue *q = rq->q;
    struct elevator_queue *e = q->elevator;
    struct blk_mq_hw_ctx *hctx = rq->mq_hctx;
    LIST_HEAD(list);

    list_add(&rq->queuelist, &list);
    e->type->ops.insert_requests(hctx, &list, at_head);

    if (run_queue)
        blk_mq_run_hw_queue(hctx, async);
}

static void
blk_mq_bio_to_request(struct request *rq, struct bio *bio,
                      unsigned int nr_segs)
{
    rq->__sector = bio->bi_iter.bi_sector;
    blk_rq_bio_prep(rq, bio, nr_segs);

    //blk_account_io_start(rq);
}

blk_qc_t blk_mq_submit_bio(struct bio *bio)
{
    struct request *rq;
    unsigned int nr_segs = 1;
    struct request_queue *q = bio->bi_disk->queue;
    struct blk_mq_alloc_data data = {
        .q = q,
    };

    data.cmd_flags = bio->bi_opf;
    rq = __blk_mq_alloc_request(&data);
    if (unlikely(!rq))
        panic("request is NULL!");

    /*
    cookie = request_to_qc_t(data.hctx, rq);
    */

    blk_mq_bio_to_request(rq, bio, nr_segs);

    /* Insert the request at the IO scheduler queue */
    blk_mq_sched_insert_request(rq, false, true, true);
    return 0;
}
EXPORT_SYMBOL(blk_mq_submit_bio);

/*
 * Softirq action handler - move entries to local list and loop over them
 * while passing them to the queue registered handler.
 */
static void blk_done_softirq(struct softirq_action *h)
{
    struct list_head local_list;

    list_replace_init(&blk_cpu_done, &local_list);

    while (!list_empty(&local_list)) {
        struct request *rq;

        rq = list_entry(local_list.next, struct request, ipi_list);
        list_del_init(&rq->ipi_list);

        rq->q->mq_ops->complete(rq);
    }
}

static void blk_mq_trigger_softirq(struct request *rq)
{
    list_add_tail(&rq->ipi_list, &blk_cpu_done);

    /*
     * If the list only contains our just added request, signal a raise of
     * the softirq.  If there are already entries there, someone already
     * raised the irq but it hasn't run yet.
     */
    if (blk_cpu_done.next == &rq->ipi_list)
        raise_softirq_irqoff(BLOCK_SOFTIRQ);
}

bool blk_mq_complete_request_remote(struct request *rq)
{
    WRITE_ONCE(rq->state, MQ_RQ_COMPLETE);

    /*
     * For a polled request, always complete locallly, it's pointless
     * to redirect the completion.
     */
    if (rq->cmd_flags & REQ_HIPRI)
        return false;

    if (rq->q->nr_hw_queues > 1)
        return false;

    blk_mq_trigger_softirq(rq);
    return true;
}
EXPORT_SYMBOL(blk_mq_complete_request_remote);

void blk_mq_complete_request(struct request *rq)
{
    BUG_ON(!blk_mq_complete_request_remote(rq));
}
EXPORT_SYMBOL(blk_mq_complete_request);

inline void __blk_mq_end_request(struct request *rq, blk_status_t error)
{
    /* Todo */
}
EXPORT_SYMBOL(__blk_mq_end_request);

void blk_mq_end_request(struct request *rq, blk_status_t error)
{
    if (blk_update_request(rq, error, blk_rq_bytes(rq)))
        BUG();
    __blk_mq_end_request(rq, error);
}
EXPORT_SYMBOL(blk_mq_end_request);

int blk_mq_init(void)
{
    INIT_LIST_HEAD(&blk_cpu_done);
    open_softirq(BLOCK_SOFTIRQ, blk_done_softirq);
    return 0;
}

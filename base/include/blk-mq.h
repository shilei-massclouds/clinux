/* SPDX-License-Identifier: GPL-2.0 */

#ifndef BLK_MQ_H
#define BLK_MQ_H

#include <blkdev.h>
#include <workqueue.h>

/**
 * enum hctx_type - Type of hardware queue
 * @HCTX_TYPE_DEFAULT:  All I/O not otherwise accounted for.
 * @HCTX_TYPE_READ: Just for READ I/O.
 * @HCTX_TYPE_POLL: Polled I/O of any kind.
 * @HCTX_MAX_TYPES: Number of types of hctx.
 */
enum hctx_type {
    HCTX_TYPE_DEFAULT,
    HCTX_TYPE_READ,
    HCTX_TYPE_POLL,

    HCTX_MAX_TYPES,
};

struct blk_mq_ctxs {
    struct kobject kobj;
    struct blk_mq_ctx *queue_ctx;
};

struct blk_mq_ctx {
    struct blk_mq_hw_ctx *hctxs[HCTX_MAX_TYPES];
    struct blk_mq_ctxs *ctxs;
};

#define queue_for_each_hw_ctx(q, hctx, i)               \
    for ((i) = 0; (i) < (q)->nr_hw_queues &&            \
         ({ hctx = (q)->queue_hw_ctx[i]; 1; }); (i)++)

enum {
    BLK_MQ_F_SHOULD_MERGE   = 1 << 0,
    BLK_MQ_F_TAG_SHARED = 1 << 1,
    /*
     * Set when this device requires underlying blk-mq device for
     * completing IO:
     */
    BLK_MQ_F_STACKING   = 1 << 2,
    BLK_MQ_F_BLOCKING   = 1 << 5,
    BLK_MQ_F_NO_SCHED   = 1 << 6,
    BLK_MQ_F_ALLOC_POLICY_START_BIT = 8,
    BLK_MQ_F_ALLOC_POLICY_BITS = 1,

    BLK_MQ_S_STOPPED    = 0,
    BLK_MQ_S_TAG_ACTIVE = 1,
    BLK_MQ_S_SCHED_RESTART  = 2,

    /* hw queue is inactive after all its CPUs become offline */
    BLK_MQ_S_INACTIVE   = 3,

    BLK_MQ_MAX_DEPTH    = 10240,

    BLK_MQ_CPU_WORK_BATCH   = 8,
};
#define BLK_MQ_FLAG_TO_ALLOC_POLICY(flags) \
    ((flags >> BLK_MQ_F_ALLOC_POLICY_START_BIT) & \
     ((1 << BLK_MQ_F_ALLOC_POLICY_BITS) - 1))

enum {
    BLK_MQ_NO_TAG       = -1U,
    BLK_MQ_TAG_MIN      = 1,
    BLK_MQ_TAG_MAX      = BLK_MQ_NO_TAG - 1,
};

struct blk_mq_tags {
    unsigned int nr_tags;
    unsigned int nr_reserved_tags;

    struct request **rqs;
    struct request **static_rqs;
    struct list_head page_list;
};

struct blk_mq_hw_ctx {
    /**
     * @state: BLK_MQ_S_* flags. Defines the state of the hw
     * queue (active, scheduled to restart, stopped).
     */
    unsigned long state;

    /**
     * @run_work: Used for scheduling a hardware queue run at a later time.
     */
    struct delayed_work run_work;

    struct blk_mq_tags *tags;
    struct blk_mq_tags *sched_tags;
    struct request_queue *queue;
};

/**
 * struct blk_mq_queue_data - Data about a request inserted in a queue
 *
 * @rq:   Request pointer.
 * @last: If it is the last request in the queue.
 */
struct blk_mq_queue_data {
    struct request *rq;
    bool last;
};

struct blk_mq_ops {
    /**
     * @queue_rq: Queue a new request from block IO.
     */
    blk_status_t (*queue_rq)(struct blk_mq_hw_ctx *,
                             const struct blk_mq_queue_data *);

    /**
     * @complete: Mark the request as complete.
     */
    void (*complete)(struct request *);

    /**
     * @init_request: Called for every command allocated by the block layer
     * to allow the driver to set up driver specific data.
     *
     * Tag greater than or equal to queue_depth is for setting up
     * flush request.
     */
    int (*init_request)(struct blk_mq_tag_set *set,
                        struct request *,
                        unsigned int);
};

struct blk_mq_tag_set {
    const struct blk_mq_ops *ops;
    unsigned int nr_hw_queues;
    unsigned int queue_depth;
    unsigned int reserved_tags;
    unsigned int cmd_size;
    unsigned int flags;
    void *driver_data;
};

struct blk_mq_alloc_data {
    struct request_queue *q;
    unsigned int cmd_flags;

    struct blk_mq_ctx *ctx;
    struct blk_mq_hw_ctx *hctx;
};

struct request_queue *
blk_mq_init_queue(struct blk_mq_tag_set *set);

struct request_queue *blk_alloc_queue();

int blk_dev_init(void);

blk_qc_t blk_mq_submit_bio(struct bio *bio);

/**
 * blk_mq_rq_to_pdu - cast a request to a PDU
 * @rq: the request to be casted
 *
 * Return: pointer to the PDU
 *
 * Driver command data is immediately after the request. So add request to get
 * the PDU.
 */
static inline void *blk_mq_rq_to_pdu(struct request *rq)
{
    return rq + 1;
}

static inline struct blk_mq_tags *
blk_mq_tags_from_data(struct blk_mq_alloc_data *data)
{
    if (data->q->elevator)
        return data->hctx->sched_tags;

    return data->hctx->tags;
}

static inline struct blk_mq_ctx *
__blk_mq_get_ctx(struct request_queue *q, unsigned int cpu)
{
    return q->queue_ctx;
}

static inline struct blk_mq_ctx *blk_mq_get_ctx(struct request_queue *q)
{
    return __blk_mq_get_ctx(q, 0);
}

static inline struct blk_mq_hw_ctx *
blk_mq_map_queue(struct request_queue *q,
                 unsigned int flags,
                 struct blk_mq_ctx *ctx)
{
    enum hctx_type type = HCTX_TYPE_DEFAULT;

    /*
     * The caller ensure that if REQ_HIPRI, poll must be enabled.
     */
    if (flags & REQ_HIPRI)
        type = HCTX_TYPE_POLL;
    else if ((flags & REQ_OP_MASK) == REQ_OP_READ)
        type = HCTX_TYPE_READ;

    return ctx->hctxs[type];
}

int
kblockd_mod_delayed_work_on(int cpu,
                            struct delayed_work *dwork,
                            unsigned long delay);

static inline struct request *blk_mq_rq_from_pdu(void *pdu)
{
    return pdu - sizeof(struct request);
}

static inline bool blk_should_fake_timeout(struct request_queue *q)
{
    return false;
}

void blk_mq_complete_request(struct request *rq);

void blk_mq_end_request(struct request *rq, blk_status_t error);

int blk_mq_init(void);

#endif /* BLK_MQ_H */

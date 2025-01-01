// SPDX-License-Identifier: GPL-2.0

#include <slab.h>
#include <errno.h>
#include <blk-mq.h>
#include <blk-mq-sched.h>
#include <export.h>

struct blk_mq_tags *
blk_mq_init_tags(unsigned int total_tags,
                 unsigned int reserved_tags,
                 int alloc_policy)
{
    struct blk_mq_tags *tags;

    if (total_tags > BLK_MQ_TAG_MAX) {
        panic("blk-mq: tag depth too large");
        return NULL;
    }

    tags = kzalloc(sizeof(*tags), GFP_KERNEL);
    if (!tags)
        return NULL;

    tags->nr_tags = total_tags;
    tags->nr_reserved_tags = reserved_tags;

    return tags;
    /* Todo */
    //return blk_mq_init_bitmap_tags(tags, node, alloc_policy);
}

struct blk_mq_tags *
blk_mq_alloc_rq_map(struct blk_mq_tag_set *set,
                    unsigned int hctx_idx,
                    unsigned int nr_tags,
                    unsigned int reserved_tags)
{
    struct blk_mq_tags *tags;

    tags = blk_mq_init_tags(nr_tags, reserved_tags,
                            BLK_MQ_FLAG_TO_ALLOC_POLICY(set->flags));
    if (!tags)
        return NULL;

    tags->rqs = kcalloc(nr_tags, sizeof(struct request *),
                        GFP_NOIO | __GFP_NOWARN | __GFP_NORETRY);
    if (!tags->rqs)
        panic("out of memory!");

    tags->static_rqs = kcalloc(nr_tags, sizeof(struct request *),
                               GFP_NOIO | __GFP_NOWARN | __GFP_NORETRY);
    if (!tags->static_rqs)
        panic("out of memory!");

    return tags;
}

static int
blk_mq_init_request(struct blk_mq_tag_set *set,
                    struct request *rq,
                    unsigned int hctx_idx)
{
    int ret;

    if (set->ops->init_request) {
        ret = set->ops->init_request(set, rq, hctx_idx);
        if (ret)
            return ret;
    }

    WRITE_ONCE(rq->state, MQ_RQ_IDLE);
    return 0;
}

static size_t order_to_size(unsigned int order)
{
    return (size_t)PAGE_SIZE << order;
}

int
blk_mq_alloc_rqs(struct blk_mq_tag_set *set,
                 struct blk_mq_tags *tags,
                 unsigned int hctx_idx,
                 unsigned int depth)
{
    size_t rq_size, left;
    unsigned int i, j, entries_per_page, max_order = 4;

    INIT_LIST_HEAD(&tags->page_list);

    /*
     * rq_size is the size of the request plus driver payload, rounded
     * to the cacheline size
     */
    rq_size = round_up(sizeof(struct request) + set->cmd_size,
                       cache_line_size());

    left = rq_size * depth;

    for (i = 0; i < depth; ) {
        void *p;
        int to_do;
        struct page *page;
        int this_order = max_order;

        while (this_order && left < order_to_size(this_order - 1))
            this_order--;

        do {
            page = alloc_pages(GFP_NOIO|__GFP_NOWARN|
                               __GFP_NORETRY|__GFP_ZERO,
                               this_order);
            if (page)
                break;
            if (!this_order--)
                break;
            if (order_to_size(this_order) < rq_size)
                break;
        } while (1);

        if (!page)
            panic("out of memory!");

        page->private = this_order;
        list_add_tail(&page->lru, &tags->page_list);

        p = page_address(page);

        entries_per_page = order_to_size(this_order) / rq_size;
        to_do = min(entries_per_page, depth - i);
        for (j = 0; j < to_do; j++) {
            struct request *rq = p;

            tags->static_rqs[i] = rq;
            if (blk_mq_init_request(set, rq, hctx_idx))
                panic("bad request!");

            p += rq_size;
            i++;
        }
    }
    return 0;
}

static int
blk_mq_sched_alloc_tags(struct request_queue *q,
                        struct blk_mq_hw_ctx *hctx,
                        unsigned int hctx_idx)
{
    int ret;
    struct blk_mq_tag_set *set = q->tag_set;

    printk("%s: step1 (%p)\n", __func__, hctx);
    hctx->sched_tags =
        blk_mq_alloc_rq_map(set, hctx_idx, q->nr_requests,
                            set->reserved_tags);
    if (!hctx->sched_tags)
        return -ENOMEM;

    ret = blk_mq_alloc_rqs(set, hctx->sched_tags, hctx_idx, q->nr_requests);
    if (ret)
        panic("bad alloc requests!");

    return ret;
}

int blk_mq_init_sched(struct request_queue *q, struct elevator_type *e)
{
    int ret;
    unsigned int i;
    struct blk_mq_hw_ctx *hctx;

    BUG_ON(!e);

    q->nr_requests = 2 * min_t(unsigned int,
                               q->tag_set->queue_depth,
                               BLKDEV_MAX_RQ);

    queue_for_each_hw_ctx(q, hctx, i) {
        ret = blk_mq_sched_alloc_tags(q, hctx, i);
        if (ret)
            panic("can not alloc tags!");
    }

    ret = e->ops.init_sched(q, e);
    if (ret)
        panic("can not init sched!");

    return 0;
}
EXPORT_SYMBOL(blk_mq_init_sched);

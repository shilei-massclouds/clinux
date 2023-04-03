/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_BLKDEV_H
#define _LINUX_BLKDEV_H

#include <fs.h>
#include <types.h>
#include <genhd.h>
#include <blk_types.h>
#include <scatterlist.h>

#define BDEVNAME_SIZE   32  /* Largest string for a blockdev identifier */
#define BLKDEV_MAX_RQ   128 /* Default maximum */

#define BLKDEV_MAJOR_MAX    512

#define rq_data_dir(rq) (op_is_write(req_op(rq)) ? WRITE : READ)

typedef unsigned int blk_qc_t;

struct block_device {
    dev_t bd_dev;  /* not a kdev_t - it's a search key */
    struct super_block *bd_super;
    struct inode *bd_inode;     /* will die */
    struct gendisk *bd_disk;
    struct backing_dev_info *bd_bdi;
    struct hd_struct *bd_part;
    u8 bd_partno;
    void *bd_claiming;
};

struct block_device_operations {
    blk_qc_t (*submit_bio) (struct bio *bio);
    int (*open)(struct block_device *, fmode_t);
    int (*rw_page)(struct block_device *, sector_t, struct page *, unsigned int);
    /*
    void (*release) (struct gendisk *, fmode_t);
    int (*ioctl) (struct block_device *, fmode_t, unsigned, unsigned long);
    int (*compat_ioctl) (struct block_device *, fmode_t, unsigned, unsigned long);
    unsigned int (*check_events) (struct gendisk *disk,
                      unsigned int clearing);
    void (*unlock_native_capacity) (struct gendisk *);
    int (*revalidate_disk) (struct gendisk *);
    int (*getgeo)(struct block_device *, struct hd_geometry *);
    void (*swap_slot_free_notify) (struct block_device *, unsigned long);
    int (*report_zones)(struct gendisk *, sector_t sector,
            unsigned int nr_zones, report_zones_cb cb, void *data);
    char *(*devnode)(struct gendisk *disk, umode_t *mode);
    struct module *owner;
    const struct pr_ops *pr_ops;
    */
};

struct queue_limits {
    unsigned int        physical_block_size;
    unsigned int        logical_block_size;
    unsigned short      max_segments;
};

struct request_queue {
    const struct blk_mq_ops *mq_ops;

    struct elevator_queue *elevator;

    struct queue_limits limits;

    /* sw queues */
    struct blk_mq_ctx *queue_ctx;

    struct blk_mq_hw_ctx **queue_hw_ctx;
    unsigned int nr_hw_queues;

    struct backing_dev_info *backing_dev_info;

    /*
     * various queue flags, see QUEUE_* below
     */
    unsigned long queue_flags;

    void *queuedata;

    struct blk_mq_tag_set *tag_set;

    unsigned long nr_requests;    /* Max # of requests */
};

/*
 * Request state for blk-mq.
 */
enum mq_rq_state {
    MQ_RQ_IDLE      = 0,
    MQ_RQ_IN_FLIGHT = 1,
    MQ_RQ_COMPLETE  = 2,
};

/*
 * request flags
 */
typedef u32 req_flags_t;

/* elevator knows about this request */
#define RQF_SORTED      ((__force req_flags_t)(1 << 0))
/* drive already may have started this one */
#define RQF_STARTED     ((__force req_flags_t)(1 << 1))
/* may not be passed by ioscheduler */
#define RQF_SOFTBARRIER     ((__force req_flags_t)(1 << 3))
/* request for flush sequence */
#define RQF_FLUSH_SEQ       ((__force req_flags_t)(1 << 4))
/* merge of different types, fail separately */
#define RQF_MIXED_MERGE     ((__force req_flags_t)(1 << 5))
/* track inflight for MQ */
#define RQF_MQ_INFLIGHT     ((__force req_flags_t)(1 << 6))
/* don't call prep for this one */
#define RQF_DONTPREP        ((__force req_flags_t)(1 << 7))
/* set for "ide_preempt" requests and also for requests for which the SCSI
   "quiesce" state must be ignored. */
#define RQF_PREEMPT     ((__force req_flags_t)(1 << 8))
/* vaguely specified driver internal error.  Ignored by the block layer */
#define RQF_FAILED      ((__force req_flags_t)(1 << 10))
/* don't warn about errors */
#define RQF_QUIET       ((__force req_flags_t)(1 << 11))
/* elevator private data attached */
#define RQF_ELVPRIV     ((__force req_flags_t)(1 << 12))
/* account into disk and partition IO statistics */
#define RQF_IO_STAT     ((__force req_flags_t)(1 << 13))
/* request came from our alloc pool */
#define RQF_ALLOCED     ((__force req_flags_t)(1 << 14))
/* runtime pm request */
#define RQF_PM          ((__force req_flags_t)(1 << 15))
/* on IO scheduler merge hash */
#define RQF_HASHED      ((__force req_flags_t)(1 << 16))
/* track IO completion time */
#define RQF_STATS       ((__force req_flags_t)(1 << 17))
/* Look at ->special_vec for the actual data payload instead of the
   bio chain. */
#define RQF_SPECIAL_PAYLOAD ((__force req_flags_t)(1 << 18))
/* The per-zone write lock is held for this request */
#define RQF_ZONE_WRITE_LOCKED   ((__force req_flags_t)(1 << 19))
/* already slept for hybrid poll */
#define RQF_MQ_POLL_SLEPT   ((__force req_flags_t)(1 << 20))
/* ->timeout has been called, don't expire again */
#define RQF_TIMED_OUT       ((__force req_flags_t)(1 << 21))

/* flags that prevent us from merging requests: */
#define RQF_NOMERGE_FLAGS \
    (RQF_STARTED | RQF_SOFTBARRIER | RQF_FLUSH_SEQ | RQF_SPECIAL_PAYLOAD)

struct request {
    struct request_queue *q;
    struct blk_mq_ctx *mq_ctx;
    struct blk_mq_hw_ctx *mq_hctx;

    unsigned int cmd_flags;     /* op and common flags */
    req_flags_t rq_flags;

    unsigned int __data_len;    /* total data len */
    sector_t __sector;          /* sector cursor */

    struct bio *bio;
    struct bio *biotail;

    struct list_head ipi_list;

    unsigned short ioprio;

    enum mq_rq_state state;
    struct list_head queuelist;

    struct gendisk *rq_disk;

    /*
     * The rb_node is only used inside the io scheduler, requests
     * are pruned when moved to the dispatch queue. So let the
     * completion_data share space with the rb_node.
     */
    union {
        struct rb_node rb_node; /* sort/lookup */
        struct bio_vec special_vec;
        void *completion_data;
        int error_count; /* for legacy drivers, don't use */
    };

    /*
     * Number of scatter-gather DMA addr+len pairs after
     * physical address coalescing is performed.
     */
    unsigned short nr_phys_segments;
};

void
blk_queue_flag_set(unsigned int flag, struct request_queue *q);

void
blk_queue_flag_clear(unsigned int flag, struct request_queue *q);

void
blk_queue_max_segments(struct request_queue *q,
                       unsigned short max_segments);

static inline unsigned
queue_logical_block_size(const struct request_queue *q)
{
    int retval = 512;

    if (q && q->limits.logical_block_size)
        retval = q->limits.logical_block_size;

    return retval;
}

static inline struct request_queue *
bdev_get_queue(struct block_device *bdev)
{
    return bdev->bd_disk->queue;    /* this is never NULL */
}

static inline unsigned int
bdev_logical_block_size(struct block_device *bdev)
{
    return queue_logical_block_size(bdev_get_queue(bdev));
}

/* assumes size > 256 */
static inline unsigned int
blksize_bits(unsigned int size)
{
    unsigned int bits = 8;
    do {
        bits++;
        size >>= 1;
    } while (size > 256);
    return bits;
}

static inline unsigned int
block_size(struct block_device *bdev)
{
    return 1 << bdev->bd_inode->i_blkbits;
}

static inline unsigned short req_get_ioprio(struct request *req)
{
    return req->ioprio;
}

static inline sector_t blk_rq_pos(const struct request *rq)
{
    return rq->__sector;
}

int
__blk_rq_map_sg(struct request_queue *q,
                struct request *rq,
                struct scatterlist *sglist,
                struct scatterlist **last_sg);

static inline int
blk_rq_map_sg(struct request_queue *q, struct request *rq,
              struct scatterlist *sglist)
{
    struct scatterlist *last_sg = NULL;

    return __blk_rq_map_sg(q, rq, sglist, &last_sg);
}

#define for_each_bio(_bio) \
    for (; _bio; _bio = _bio->bi_next)

static inline unsigned int blk_rq_bytes(const struct request *rq)
{
    return rq->__data_len;
}

static inline sector_t get_start_sect(struct block_device *bdev)
{
    return bdev->bd_part->start_sect;
}

/*
 * Number of physical segments as sent to the device.
 *
 * Normally this is the number of discontiguous data segments sent by the
 * submitter.  But for data-less command like discard we might have no
 * actual data segments submitted, but the driver might have to add it's
 * own special payload.  In that case we still return 1 here so that this
 * special payload will be mapped.
 */
static inline unsigned short blk_rq_nr_phys_segments(struct request *rq)
{
    if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
        return 1;
    return rq->nr_phys_segments;
}

#endif /* _LINUX_BLKDEV_H */

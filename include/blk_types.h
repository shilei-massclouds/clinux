/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BLK_TYPES_H
#define __LINUX_BLK_TYPES_H

#include <bvec.h>

#define BVEC_POOL_NR    6
#define BVEC_POOL_MAX   (BVEC_POOL_NR - 1)

/*
 * Top 3 bits of bio flags indicate the pool the bvecs came from.  We add
 * 1 to the actual index so that 0 indicates that there are no bvecs to be
 * freed.
 */
#define BVEC_POOL_BITS      (3)
#define BVEC_POOL_OFFSET    (16 - BVEC_POOL_BITS)

#define REQ_OP_BITS 8
#define REQ_OP_MASK ((1 << REQ_OP_BITS) - 1)

struct bio;
typedef void (bio_end_io_t)(struct bio *);

enum req_opf {
    /* read sectors from the device */
    REQ_OP_READ     = 0,
    /* write sectors to the device */
    REQ_OP_WRITE    = 1,
    /* flush the volatile write cache */
    REQ_OP_FLUSH    = 2,
    /* discard sectors */
    REQ_OP_DISCARD  = 3,
    /* securely erase sectors */
    REQ_OP_SECURE_ERASE = 5,
    /* write the same sector many times */
    REQ_OP_WRITE_SAME   = 7,
    /* write the zero filled sector many times */
    REQ_OP_WRITE_ZEROES = 9,

    REQ_OP_LAST,
};

typedef u8 blk_status_t;
#define BLK_STS_OK 0
#define BLK_STS_NOTSUPP     ((__force blk_status_t)1)
#define BLK_STS_TIMEOUT     ((__force blk_status_t)2)
#define BLK_STS_NOSPC       ((__force blk_status_t)3)
#define BLK_STS_TRANSPORT   ((__force blk_status_t)4)
#define BLK_STS_TARGET      ((__force blk_status_t)5)
#define BLK_STS_NEXUS       ((__force blk_status_t)6)
#define BLK_STS_MEDIUM      ((__force blk_status_t)7)
#define BLK_STS_PROTECTION  ((__force blk_status_t)8)
#define BLK_STS_RESOURCE    ((__force blk_status_t)9)
#define BLK_STS_IOERR       ((__force blk_status_t)10)
/* hack for device mapper, don't use elsewhere: */
#define BLK_STS_DM_REQUEUE  ((__force blk_status_t)11)
#define BLK_STS_AGAIN       ((__force blk_status_t)12)

#define BLK_STS_DEV_RESOURCE    ((__force blk_status_t)13)
#define BLK_STS_ZONE_RESOURCE   ((__force blk_status_t)14)

#define bio_op(bio) ((bio)->bi_opf & REQ_OP_MASK)
#define req_op(req) ((req)->cmd_flags & REQ_OP_MASK)

enum req_flag_bits {
    __REQ_FAILFAST_DEV =    /* no driver retries of device errors */
        REQ_OP_BITS,
    __REQ_FAILFAST_TRANSPORT, /* no driver retries of transport errors */
    __REQ_FAILFAST_DRIVER,  /* no driver retries of driver errors */
    __REQ_SYNC,     /* request is sync (sync write or read) */
    __REQ_META,     /* metadata io request */
    __REQ_PRIO,     /* boost priority in cfq */
    __REQ_NOMERGE,      /* don't touch this for merging */
    __REQ_IDLE,     /* anticipate more IO after this one */
    __REQ_INTEGRITY,    /* I/O includes block integrity payload */
    __REQ_FUA,      /* forced unit access */
    __REQ_PREFLUSH,     /* request for cache flush */
    __REQ_RAHEAD,       /* read ahead, can fail anytime */
    __REQ_BACKGROUND,   /* background IO */
    __REQ_NOWAIT,           /* Don't wait if request will block */
    /*
     * When a shared kthread needs to issue a bio for a cgroup, doing
     * so synchronously can lead to priority inversions as the kthread
     * can be trapped waiting for that cgroup.  CGROUP_PUNT flag makes
     * submit_bio() punt the actual issuing to a dedicated per-blkcg
     * work item to avoid such priority inversions.
     */
    __REQ_CGROUP_PUNT,

    /* command specific flags for REQ_OP_WRITE_ZEROES: */
    __REQ_NOUNMAP,      /* do not free blocks when zeroing */

    __REQ_HIPRI,

    /* for driver use */
    __REQ_DRV,
    __REQ_SWAP,     /* swapping request. */
    __REQ_NR_BITS,      /* stops here */
};

#define REQ_RAHEAD      (1ULL << __REQ_RAHEAD)
#define REQ_HIPRI       (1ULL << __REQ_HIPRI)

/*
 * bio flags
 */
enum {
    BIO_NO_PAGE_REF,    /* don't put release vec pages */
    BIO_CLONED,         /* doesn't own data */
    BIO_BOUNCED,        /* bio is a bounce bio */
    BIO_USER_MAPPED,    /* contains user pages */
    BIO_NULL_MAPPED,    /* contains invalid user pages */
    BIO_WORKINGSET,     /* contains userspace workingset pages */
    BIO_QUIET,          /* Make BIO Quiet */
    BIO_CHAIN,          /* chained bio, ->bi_remaining in effect */
    BIO_REFFED,         /* bio has elevated ->bi_cnt */
    BIO_THROTTLED,      /* This bio has already been subjected to
                         * throttling rules. Don't do it again. */
    BIO_TRACE_COMPLETION,   /* bio_endio() should trace the final
                               completion of this bio. */
    BIO_CGROUP_ACCT,    /* has been accounted to a cgroup */
    BIO_TRACKED,        /* set if bio goes through the rq_qos path */
    BIO_FLAG_LAST
};

struct bio {
    struct bio *bi_next;    /* request queue link */

    struct gendisk *bi_disk;

    /* bottom bits req flags, top bits REQ_OP.
     * Use accessors. */
    unsigned int bi_opf;

    unsigned short bi_flags;    /* status, etc and bvec pool number */
    unsigned short bi_ioprio;

    bio_end_io_t *bi_end_io;
    void *bi_private;

    blk_status_t bi_status;

    struct bvec_iter bi_iter;

    u8 bi_partno;

    struct bio_set *bi_pool;

    unsigned short bi_vcnt;     /* how many bio_vec's */

    unsigned short bi_max_vecs; /* max bvl_vecs we can hold */

    struct bio_vec *bi_io_vec;  /* the actual vec list */

    struct bio_vec bi_inline_vecs[];
};

/* obsolete, don't use in new code */
static inline void
bio_set_op_attrs(struct bio *bio, unsigned op, unsigned op_flags)
{
    bio->bi_opf = op | op_flags;
}

static inline bool op_is_write(unsigned int op)
{
    return (op & 1);
}

#endif /* __LINUX_BLK_TYPES_H */

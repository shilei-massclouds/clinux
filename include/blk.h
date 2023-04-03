/* SPDX-License-Identifier: GPL-2.0 */
#ifndef BLK_INTERNAL_H
#define BLK_INTERNAL_H

#include <bio.h>
#include <blkdev.h>

static inline void
blk_rq_bio_prep(struct request *rq, struct bio *bio,
                unsigned int nr_segs)
{
    rq->nr_phys_segments = nr_segs;
    rq->__data_len = bio->bi_iter.bi_size;
    rq->bio = rq->biotail = bio;
    rq->ioprio = bio_prio(bio);

    if (bio->bi_disk)
        rq->rq_disk = bio->bi_disk;
}

bool blk_update_request(struct request *req, blk_status_t error,
                        unsigned int nr_bytes);

#endif /* BLK_INTERNAL_H */

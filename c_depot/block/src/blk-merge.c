// SPDX-License-Identifier: GPL-2.0

#include <bio.h>
#include <blkdev.h>
#include <export.h>
#include <scatterlist.h>

static inline struct scatterlist *
blk_next_sg(struct scatterlist **sg,
            struct scatterlist *sglist)
{
    if (!*sg)
        return sglist;

    sg_unmark_end(*sg);
    return sg_next(*sg);
}

static inline int
__blk_bvec_map_sg(struct bio_vec bv,
                  struct scatterlist *sglist,
                  struct scatterlist **sg)
{
    *sg = blk_next_sg(sg, sglist);
    sg_set_page(*sg, bv.bv_page, bv.bv_len, bv.bv_offset);
    return 1;
}

static int
__blk_bios_map_sg(struct request_queue *q,
                  struct bio *bio,
                  struct scatterlist *sglist,
                  struct scatterlist **sg)
{
    struct bio_vec bvec;
    struct bvec_iter iter;
    int nsegs = 0;

    for_each_bio(bio) {
        bio_for_each_bvec(bvec, bio, iter) {
            if (bvec.bv_offset + bvec.bv_len <= PAGE_SIZE)
                nsegs += __blk_bvec_map_sg(bvec, sglist, sg);
            else
                panic("over PAGE_SIZE!");
        }
    }

    return nsegs;
}

int
__blk_rq_map_sg(struct request_queue *q, struct request *rq,
                struct scatterlist *sglist, struct scatterlist **last_sg)
{
    int nsegs = 0;

    if (rq->rq_flags & RQF_SPECIAL_PAYLOAD)
        nsegs = __blk_bvec_map_sg(rq->special_vec, sglist, last_sg);
    else if (rq->bio && bio_op(rq->bio) == REQ_OP_WRITE_SAME)
        nsegs = __blk_bvec_map_sg(bio_iovec(rq->bio), sglist, last_sg);
    else if (rq->bio) {
        nsegs = __blk_bios_map_sg(q, rq->bio, sglist, last_sg);
    }

    if (*last_sg)
        sg_mark_end(*last_sg);

    /*
     * Something must have been wrong if the figured number of
     * segment is bigger than number of req's physical segments
     */
    if (nsegs > blk_rq_nr_phys_segments(rq)) {
        pr_warn("%s: %u > %u\n", __func__, nsegs, blk_rq_nr_phys_segments(rq));
    }

    return nsegs;
}
EXPORT_SYMBOL(__blk_rq_map_sg);


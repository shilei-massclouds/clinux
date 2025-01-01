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

    BUG_ON(rq->bio == NULL);
    nsegs = __blk_bios_map_sg(q, rq->bio, sglist, last_sg);

    BUG_ON(*last_sg == NULL);
    sg_mark_end(*last_sg);
    return nsegs;
}
EXPORT_SYMBOL(__blk_rq_map_sg);


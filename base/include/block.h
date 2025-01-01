/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <blkdev.h>
#include <blk_types.h>

void
blk_queue_logical_block_size(struct request_queue *q, unsigned int size);

blk_qc_t submit_bio(struct bio *bio);

int blk_status_to_errno(blk_status_t status);

#endif /* _BLOCK_H_ */

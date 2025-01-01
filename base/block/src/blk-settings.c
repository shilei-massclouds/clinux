// SPDX-License-Identifier: GPL-2.0

#include <blkdev.h>
#include <export.h>

void
blk_queue_logical_block_size(struct request_queue *q, unsigned int size)
{
    q->limits.logical_block_size = size;

    if (q->limits.physical_block_size < size)
        q->limits.physical_block_size = size;
}
EXPORT_SYMBOL(blk_queue_logical_block_size);

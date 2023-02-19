// SPDX-License-Identifier: GPL-2.0+

#include <block.h>
#include <blk-mq.h>
#include <export.h>
#include <printk.h>

extern int blk_dev_init(void);
extern int deadline_init(void);

void
blk_queue_flag_set(unsigned int flag, struct request_queue *q)
{
    set_bit(flag, &q->queue_flags);
}
EXPORT_SYMBOL(blk_queue_flag_set);

void
blk_queue_flag_clear(unsigned int flag, struct request_queue *q)
{
    clear_bit(flag, &q->queue_flags);
}
EXPORT_SYMBOL(blk_queue_flag_clear);

void
wbt_set_write_cache(struct request_queue *q, bool write_cache_on)
{
    /* Todo: */
}

void
blk_queue_max_segments(struct request_queue *q,
                       unsigned short max_segments)
{
    if (!max_segments) {
        max_segments = 1;
        printk("%s: set to minimum %d\n", __func__, max_segments);
    }

    q->limits.max_segments = max_segments;
}
EXPORT_SYMBOL(blk_queue_max_segments);

int
init_module(void)
{
    printk("module[block]: init begin ...\n");

    blk_mq_init();
    deadline_init();
    blk_dev_init();

    printk("module[block]: init end!\n");
    return 0;
}

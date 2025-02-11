// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/ktime.h>
#include <linux/timerqueue.h>
#include "../../booter/src/booter.h"

int
cl_hrtimer_init(void)
{
    sbi_puts("module[hrtimer]: init begin ...\n");
    sbi_puts("module[hrtimer]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_hrtimer_init);

int tick_program_event(ktime_t expires, int force)
{
    booter_panic("No impl 'tick_program_event'.");
}

bool timerqueue_del(struct timerqueue_head *head, struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_del'.");
}

struct timerqueue_node *timerqueue_iterate_next(struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_iterate_next'.");
}

ktime_t ktime_get(void)
{
    booter_panic("No impl 'ktime_get'.");
}
EXPORT_SYMBOL(ktime_get);

ktime_t ktime_get_with_offset(enum tk_offsets offs)
{
    booter_panic("No impl 'ktime_get_with_offset'.");
}

bool timerqueue_add(struct timerqueue_head *head, struct timerqueue_node *node)
{
    booter_panic("No impl 'timerqueue_add'.");
}

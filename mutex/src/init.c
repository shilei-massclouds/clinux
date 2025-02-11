// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/wake_q.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_mutex_init(void)
{
    sbi_puts("module[mutex]: init begin ...\n");
    sbi_puts("module[mutex]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_mutex_init);

/*
void wake_q_add(struct wake_q_head *head, struct task_struct *task)
{
    booter_panic("in mutex!");
}

void wake_up_q(struct wake_q_head *head)
{
    booter_panic("in mutex!");
}
*/

void __sched schedule_preempt_disabled(void)
{
    booter_panic("in mutex!");
}

/*
void io_schedule_finish(int token)
{
    booter_panic("in mutex!");
}

int io_schedule_prepare(void)
{
    booter_panic("in mutex!");
}
*/

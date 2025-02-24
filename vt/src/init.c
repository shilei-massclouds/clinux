// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/console.h>
#include <linux/interrupt.h>
#include <linux/sched/debug.h>
#include <linux/input.h>
#include <linux/moduleparam.h>
#include "../../booter/src/booter.h"

int
cl_vt_init(void)
{
    sbi_puts("module[vt]: init begin ...\n");
    sbi_puts("module[vt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_vt_init);

void ctrl_alt_del(void)
{
    booter_panic("No impl!\n");
}

void *vmemdup_user(const void __user *src, size_t len)
{
    booter_panic("No impl!\n");
}

const struct kernel_param_ops param_array_ops;

void __sched console_conditional_schedule(void)
{
    booter_panic("No impl!\n");
}

loff_t fixed_size_llseek(struct file *file, loff_t offset, int whence, loff_t size)
{
    booter_panic("No impl!\n");
}

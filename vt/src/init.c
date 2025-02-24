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

void __tasklet_schedule(struct tasklet_struct *t)
{
    booter_panic("No impl!\n");
}

int input_register_handle(struct input_handle *handle)
{
    booter_panic("No impl!\n");
}

void ctrl_alt_del(void)
{
    booter_panic("No impl!\n");
}

int input_handler_for_each_handle(struct input_handler *handler, void *data,
                  int (*fn)(struct input_handle *, void *))
{
    booter_panic("No impl!\n");
}

/*
u32 conv_8bit_to_uni(unsigned char c)
{
    booter_panic("No impl!\n");
}
*/
int input_open_device(struct input_handle *handle)
{
    booter_panic("No impl!\n");
}
void input_close_device(struct input_handle *handle)
{
    booter_panic("No impl!\n");
}
void input_inject_event(struct input_handle *handle,
            unsigned int type, unsigned int code, int value)
{
    booter_panic("No impl!\n");
}

void *vmemdup_user(const void __user *src, size_t len)
{
    booter_panic("No impl!\n");
}

void input_unregister_handle(struct input_handle *handle)
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

int input_register_handler(struct input_handler *handler)
{
    booter_panic("No impl!\n");
}


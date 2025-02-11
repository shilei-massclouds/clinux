// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/smp.h>
#include "../../booter/src/booter.h"

int
cl_slub_init(void)
{
    sbi_puts("module[slub]: init begin ...\n");
    sbi_puts("module[slub]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_slub_init);

long copy_from_kernel_nofault(void *dst, const void *src, size_t size)
{
    booter_panic("No impl 'slub'.");
}

void print_hex_dump(const char *level, const char *prefix_str, int prefix_type,
            int rowsize, int groupsize,
            const void *buf, size_t len, bool ascii)
{
    booter_panic("No impl 'slub'.");
}

unsigned int stack_trace_save(unsigned long *store, unsigned int size,
                  unsigned int skipnr)
{
    booter_panic("No impl 'slub'.");
}

void call_rcu(struct rcu_head *head, rcu_callback_t func)
{
    booter_panic("No impl 'slub'.");
}

void on_each_cpu_cond(smp_cond_func_t cond_func, smp_call_func_t func,
              void *info, bool wait)
{
    booter_panic("No impl 'slub'.");
}

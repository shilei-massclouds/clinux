// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/jump_label.h>
#include <linux/cache.h>
#include <linux/fs.h>
#include <linux/splice.h>
#include "../../booter/src/booter.h"

int
cl_net_core_init(void)
{
    sbi_puts("module[net_core]: init begin ...\n");
    sbi_puts("module[net_core]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_net_core_init);

DEFINE_STATIC_KEY_FALSE(memalloc_socks_key);
EXPORT_SYMBOL_GPL(memalloc_socks_key);

int sysctl_tstamp_allow_data __read_mostly = 1;

struct pipe_buf_operations;
const struct pipe_buf_operations nosteal_pipe_buf_ops;

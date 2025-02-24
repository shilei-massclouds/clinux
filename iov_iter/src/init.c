// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/pipe_fs_i.h>
#include "../../booter/src/booter.h"

int
cl_iov_iter_init(void)
{
    sbi_puts("module[iov_iter]: init begin ...\n");
    sbi_puts("module[iov_iter]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_iov_iter_init);

const struct pipe_buf_operations page_cache_pipe_buf_ops;
const struct pipe_buf_operations default_pipe_buf_ops;

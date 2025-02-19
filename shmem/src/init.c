// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/xattr.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_shmem_init(void)
{
    sbi_puts("module[shmem]: init begin ...\n");
    sbi_puts("module[shmem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_shmem_init);

DEFINE_ENABLE_FUNC(shmem);

const struct xattr_handler posix_acl_default_xattr_handler;
const struct xattr_handler posix_acl_access_xattr_handler;

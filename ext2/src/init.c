// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/highuid.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

int
cl_ext2_init(void)
{
    sbi_puts("module[ext2]: init begin ...\n");
    sbi_puts("module[ext2]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_ext2_init);

DEFINE_ENABLE_FUNC(ext2);

/*
 * the same as above, but for filesystems which can only store a 16-bit
 * UID and GID. as such, this is needed on all architectures
 */

int fs_overflowuid = DEFAULT_FS_OVERFLOWUID;
int fs_overflowgid = DEFAULT_FS_OVERFLOWGID;

EXPORT_SYMBOL(fs_overflowuid);
EXPORT_SYMBOL(fs_overflowgid);

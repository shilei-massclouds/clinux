/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BACKING_DEV_H
#define _LINUX_BACKING_DEV_H

#include <blkdev.h>
#include <backing-dev-defs.h>

#define BDI_CAP_NO_ACCT_DIRTY   0x00000001
#define BDI_CAP_NO_WRITEBACK    0x00000002
#define BDI_CAP_NO_ACCT_WB      0x00000004
#define BDI_CAP_STABLE_WRITES   0x00000008
#define BDI_CAP_STRICTLIMIT     0x00000010
#define BDI_CAP_CGROUP_WRITEBACK 0x00000020
#define BDI_CAP_SYNCHRONOUS_IO  0x00000040

#define BDI_CAP_NO_ACCT_AND_WRITEBACK \
    (BDI_CAP_NO_WRITEBACK | BDI_CAP_NO_ACCT_DIRTY | BDI_CAP_NO_ACCT_WB)

extern struct backing_dev_info noop_backing_dev_info;

static inline struct backing_dev_info *
inode_to_bdi(struct inode *inode)
{
    struct super_block *sb;

    if (!inode)
        return &noop_backing_dev_info;

    sb = inode->i_sb;
    if (sb_is_blkdev_sb(sb))
        return I_BDEV(inode)->bd_bdi;
    printk("########### %s s_bdi(%lx)\n", __func__, sb->s_bdi);
    return sb->s_bdi;
}

struct backing_dev_info *bdi_alloc();

#endif /* _LINUX_BACKING_DEV_H */

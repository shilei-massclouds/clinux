/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BACKING_DEV_DEFS_H
#define __LINUX_BACKING_DEV_DEFS_H

struct backing_dev_info {
    unsigned long ra_pages;     /* max readahead in PAGE_SIZE units */
    unsigned long io_pages;     /* max allowed IO size */
    unsigned int capabilities;  /* Device capabilities */
};

extern struct backing_dev_info noop_backing_dev_info;

#endif /* __LINUX_BACKING_DEV_DEFS_H */

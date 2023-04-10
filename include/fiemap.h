/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * FS_IOC_FIEMAP ioctl infrastructure.
 *
 * Some portions copyright (C) 2007 Cluster File Systems, Inc
 *
 * Authors: Mark Fasheh <mfasheh@suse.com>
 *          Kalpak Shah <kalpak.shah@sun.com>
 *          Andreas Dilger <adilger@sun.com>
 */

#ifndef _UAPI_LINUX_FIEMAP_H
#define _UAPI_LINUX_FIEMAP_H

#include <types.h>

struct fiemap_extent {
    __u64 fe_logical;  /* logical offset in bytes for the start of
                * the extent from the beginning of the file */
    __u64 fe_physical; /* physical offset in bytes for the start
                * of the extent from the beginning of the disk */
    __u64 fe_length;   /* length in bytes for this extent */
    __u64 fe_reserved64[2];
    __u32 fe_flags;    /* FIEMAP_EXTENT_* flags for this extent */
    __u32 fe_reserved[3];
};

struct fiemap {
    __u64 fm_start;     /* logical offset (inclusive) at
                 * which to start mapping (in) */
    __u64 fm_length;    /* logical length of mapping which
                 * userspace wants (in) */
    __u32 fm_flags;     /* FIEMAP_FLAG_* flags for request (in/out) */
    __u32 fm_mapped_extents;/* number of extents that were mapped (out) */
    __u32 fm_extent_count;  /* size of fm_extents array (in) */
    __u32 fm_reserved;
    struct fiemap_extent fm_extents[0]; /* array of mapped extents (out) */
};

#endif /* _UAPI_LINUX_FIEMAP_H */

/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _ASM_GENERIC_FCNTL_H
#define _ASM_GENERIC_FCNTL_H

#define O_ACCMODE   00000003
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_CREAT     00000100    /* not fcntl */
#define O_EXCL      00000200    /* not fcntl */
#define O_NOCTTY    00000400    /* not fcntl */
#define O_TRUNC     00001000    /* not fcntl */
#define O_APPEND    00002000
#define O_NONBLOCK  00004000
#define O_DSYNC     00010000    /* used to be O_SYNC, see below */
#define FASYNC      00020000    /* fcntl, for BSD compatibility */
#define O_DIRECT    00040000    /* direct disk access hint */
#define O_LARGEFILE 00100000
#define O_DIRECTORY 00200000    /* must be a directory */
#define O_NOFOLLOW  00400000    /* don't follow links */
#define O_NOATIME   01000000
#define O_CLOEXEC   02000000    /* set close_on_exec */

#define __O_SYNC    04000000

#define O_PATH      010000000
#define __O_TMPFILE 020000000

/* a horrid kludge trying to make sure that this will fail on old kernels */
#define O_TMPFILE       (__O_TMPFILE | O_DIRECTORY)
#define O_TMPFILE_MASK  (__O_TMPFILE | O_DIRECTORY | O_CREAT)

#define O_NDELAY    O_NONBLOCK

#define AT_SYMLINK_NOFOLLOW 0x100   /* Do not follow symbolic links. */
#define AT_EACCESS          0x200   /* Test access permitted for
                                       effective IDs, not real IDs. */
#define AT_EMPTY_PATH       0x1000  /* Allow empty relative pathname */

/* List of all valid flags for the open/openat flags argument: */
#define VALID_OPEN_FLAGS \
    (O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_EXCL | O_NOCTTY | \
     O_TRUNC | O_APPEND | O_NDELAY | O_NONBLOCK | O_NDELAY | \
     __O_SYNC | O_DSYNC | FASYNC | O_DIRECT | O_LARGEFILE | \
     O_DIRECTORY | O_NOFOLLOW | \
     O_NOATIME | O_CLOEXEC | O_PATH | __O_TMPFILE)

/* List of all valid flags for the how->resolve argument: */
#define VALID_RESOLVE_FLAGS \
    (RESOLVE_NO_XDEV | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_SYMLINKS | \
     RESOLVE_BENEATH | RESOLVE_IN_ROOT)

#endif /* _ASM_GENERIC_FCNTL_H */

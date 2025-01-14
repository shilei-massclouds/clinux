/* SPDX-License-Identifier: GPL-2.0 */
/*
 * descriptor table internals; you almost certainly want file.h instead.
 */

#ifndef __LINUX_FDTABLE_H
#define __LINUX_FDTABLE_H

/*
 * The default fd array needs to be at least BITS_PER_LONG,
 * as this is the granularity returned by copy_fdset().
 */
#define NR_OPEN_DEFAULT BITS_PER_LONG

struct fdtable {
    unsigned int max_fds;
    struct file **fd;               /* current fd array */
    unsigned long *close_on_exec;
    unsigned long *open_fds;
    unsigned long *full_fds_bits;
};

/*
 * Open file table structure
 */
struct files_struct {
    struct fdtable *fdt;
    struct fdtable fdtab;
    unsigned int next_fd;
    unsigned long close_on_exec_init[1];
    unsigned long open_fds_init[1];
    unsigned long full_fds_bits_init[1];
    struct file *fd_array[NR_OPEN_DEFAULT];
};

#define files_fdtable(files) (files)->fdt

/*
 * The caller must ensure that fd table isn't shared or hold rcu or file lock
 */
static inline struct file *
__fcheck_files(struct files_struct *files, unsigned int fd)
{
    struct fdtable *fdt = files->fdt;

    if (fd < fdt->max_fds)
        return fdt->fd[fd];
    return NULL;
}

static inline struct file *
fcheck_files(struct files_struct *files, unsigned int fd)
{
    return __fcheck_files(files, fd);
}

#endif /* __LINUX_FDTABLE_H */

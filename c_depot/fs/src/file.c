/* SPDX-License-Identifier: GPL-2.0 */

#include <fs.h>
#include <file.h>
#include <export.h>
#include <signal.h>
#include <current.h>
#include <fdtable.h>
#include <find_bit.h>
#include <resource.h>

static unsigned int
find_next_fd(struct fdtable *fdt, unsigned int start)
{
    unsigned int maxfd = fdt->max_fds;
    unsigned int maxbit = maxfd / BITS_PER_LONG;
    unsigned int bitbit = start / BITS_PER_LONG;

    bitbit = find_next_zero_bit(fdt->full_fds_bits, maxbit, bitbit) * BITS_PER_LONG;
    if (bitbit > maxfd)
        return maxfd;
    if (bitbit > start)
        start = bitbit;
    return find_next_zero_bit(fdt->open_fds, maxfd, start);
}

static int expand_files(struct files_struct *files, unsigned int nr)
{
    struct fdtable *fdt;
    int expanded = 0;

    fdt = files_fdtable(files);

    /* Do we need to expand? */
    if (nr < fdt->max_fds)
        return expanded;

    panic("%s: need expand files table!", __func__);
}

static inline void __set_close_on_exec(unsigned int fd, struct fdtable *fdt)
{
    __set_bit(fd, fdt->close_on_exec);
}

static inline void __clear_close_on_exec(unsigned int fd, struct fdtable *fdt)
{
    if (test_bit(fd, fdt->close_on_exec))
        __clear_bit(fd, fdt->close_on_exec);
}

static inline void __set_open_fd(unsigned int fd, struct fdtable *fdt)
{
    __set_bit(fd, fdt->open_fds);
    fd /= BITS_PER_LONG;
    if (!~fdt->open_fds[fd])
        __set_bit(fd, fdt->full_fds_bits);
}

/*
 * allocate a file descriptor, mark it busy.
 */
int __alloc_fd(struct files_struct *files,
               unsigned start, unsigned end, unsigned flags)
{
    int error;
    unsigned int fd;
    struct fdtable *fdt;

    fdt = files_fdtable(files);
    fd = start;
    if (fd < files->next_fd)
        fd = files->next_fd;

    if (fd < fdt->max_fds)
        fd = find_next_fd(fdt, fd);

    printk("%s: fd(%u), end(%u)\n", __func__, fd, end);
    /*
     * N.B. For clone tasks sharing a files structure, this test
     * will limit the total number of files that can be opened.
     */
    if (fd >= end)
        return -EMFILE;

    error = expand_files(files, fd);
    if (error < 0)
        return error;

    if (start <= files->next_fd)
        files->next_fd = fd + 1;

    __set_open_fd(fd, fdt);
    if (flags & O_CLOEXEC)
        __set_close_on_exec(fd, fdt);
    else
        __clear_close_on_exec(fd, fdt);

    return fd;
}

int __get_unused_fd_flags(unsigned flags, unsigned long nofile)
{
    return __alloc_fd(current->files, 0, nofile, flags);
}

int get_unused_fd_flags(unsigned flags)
{
    return __get_unused_fd_flags(flags, rlimit(RLIMIT_NOFILE));
}
EXPORT_SYMBOL(get_unused_fd_flags);

void
__fd_install(struct files_struct *files,
             unsigned int fd, struct file *file)
{
    struct fdtable *fdt;

    fdt = files->fdt;
    BUG_ON(fdt->fd[fd] != NULL);
    fdt->fd[fd] = file;
}

/*
 * This consumes the "file" refcount, so callers should treat it
 * as if they had called fput(file).
 */
void fd_install(unsigned int fd, struct file *file)
{
    __fd_install(current->files, fd, file);
}
EXPORT_SYMBOL(fd_install);

static struct file *
__fget_files(struct files_struct *files, unsigned int fd, fmode_t mask,
             unsigned int refs)
{
    struct file *file;

    pr_info("%s: 0 files(%p)\n", __func__, files);
    file = fcheck_files(files, fd);
    if (file) {
        /* File object ref couldn't be taken.
         * dup2() atomicity guarantee is the reason
         * we loop to catch the new file (or NULL pointer)
         */
        if (file->f_mode & mask)
            file = NULL;
    }

    return file;
}

static inline struct file *
__fget(unsigned int fd, fmode_t mask, unsigned int refs)
{
    return __fget_files(current->files, fd, mask, refs);
}

static unsigned long __fget_light(unsigned int fd, fmode_t mask)
{
    struct file *file;

    file = __fget(fd, mask, 1);
    if (!file)
        return 0;
    return FDPUT_FPUT | (unsigned long)file;
}

unsigned long __fdget(unsigned int fd)
{
    return __fget_light(fd, FMODE_PATH);
}
EXPORT_SYMBOL(__fdget);

unsigned long __fdget_pos(unsigned int fd)
{
    unsigned long v = __fdget(fd);
    return v;
}

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __LINUX_FILE_H
#define __LINUX_FILE_H

#include <fs.h>

struct fd {
    struct file *file;
    unsigned int flags;
};

#define FDPUT_FPUT       1
#define FDPUT_POS_UNLOCK 2

extern unsigned long __fdget_pos(unsigned int fd);
extern unsigned long __fdget(unsigned int fd);

int get_unused_fd_flags(unsigned flags);

void fd_install(unsigned int fd, struct file *file);

static inline struct fd __to_fd(unsigned long v)
{
    return (struct fd){(struct file *)(v & ~3),v & 3};
}

static inline struct fd fdget(unsigned int fd)
{
    return __to_fd(__fdget(fd));
}

static inline struct fd fdget_pos(int fd)
{
    return __to_fd(__fdget_pos(fd));
}

static inline void fdput(struct fd fd)
{
#if 0
    if (fd.flags & FDPUT_FPUT)
        fput(fd.file);
#endif
}

static inline void fdput_pos(struct fd f)
{
#if 0
    if (f.flags & FDPUT_POS_UNLOCK)
        __f_unlock_pos(f.file);
    fdput(f);
#endif
}

#endif /* __LINUX_FILE_H */

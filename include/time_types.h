/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_TIME_TYPES_H
#define _UAPI_LINUX_TIME_TYPES_H

#include <types.h>

struct __kernel_timespec {
    __kernel_time64_t       tv_sec;                 /* seconds */
    long long               tv_nsec;                /* nanoseconds */
};

#endif /* _UAPI_LINUX_TIME_TYPES_H */

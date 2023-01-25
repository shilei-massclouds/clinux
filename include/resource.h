/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_RESOURCE_H
#define _UAPI_LINUX_RESOURCE_H

#include <types.h>

/*
 * Limit the stack by to some sane default: root can always
 * increase this limit if needed..  8MB seems reasonable.
 */
#define _STK_LIM    (8*1024*1024)

# define RLIM_INFINITY  (~0UL)

#define RLIMIT_CPU      0   /* CPU time in sec */
#define RLIMIT_FSIZE    1   /* Maximum filesize */
#define RLIMIT_DATA     2   /* max data size */
#define RLIMIT_STACK    3   /* max stack size */
#define RLIMIT_CORE     4   /* max core file size */
#ifndef RLIMIT_RSS
# define RLIMIT_RSS     5   /* max resident set size */
#endif

#ifndef RLIMIT_NPROC
# define RLIMIT_NPROC   6   /* max number of processes */
#endif

#ifndef RLIMIT_NOFILE
# define RLIMIT_NOFILE  7   /* max number of open files */
#endif

#ifndef RLIMIT_MEMLOCK
# define RLIMIT_MEMLOCK 8   /* max locked-in-memory address space */
#endif

#ifndef RLIMIT_AS
# define RLIMIT_AS      9   /* address space limit */
#endif

#define RLIMIT_LOCKS        10  /* maximum file locks held */
#define RLIMIT_SIGPENDING   11  /* max number of pending signals */
#define RLIMIT_MSGQUEUE     12  /* maximum bytes in POSIX mqueues */
#define RLIMIT_NICE     13  /* max nice prio allowed to raise to
                               0-39 for nice level 19 .. -20 */
#define RLIMIT_RTPRIO   14  /* maximum realtime priority */
#define RLIMIT_RTTIME   15  /* timeout for RT tasks in us */
#define RLIM_NLIMITS    16

/*
 * boot-time rlimit defaults for the init task:
 */
#define INIT_RLIMITS                            \
{                                   \
    [RLIMIT_CPU]        = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_FSIZE]      = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_DATA]       = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_STACK]      = {       _STK_LIM,  RLIM_INFINITY },   \
    [RLIMIT_CORE]       = {              0,  RLIM_INFINITY },   \
    [RLIMIT_RSS]        = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_NPROC]      = {              0,              0 },   \
    [RLIMIT_NOFILE]     = {   INR_OPEN_CUR,   INR_OPEN_MAX },   \
    [RLIMIT_AS]         = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_LOCKS]      = {  RLIM_INFINITY,  RLIM_INFINITY },   \
    [RLIMIT_SIGPENDING] = {         0,         0 }, \
    [RLIMIT_NICE]       = { 0, 0 },             \
    [RLIMIT_RTPRIO]     = { 0, 0 },             \
    [RLIMIT_RTTIME]     = {  RLIM_INFINITY,  RLIM_INFINITY },   \
}

struct rlimit {
    __kernel_ulong_t    rlim_cur;
    __kernel_ulong_t    rlim_max;
};

#endif /* _UAPI_LINUX_RESOURCE_H */

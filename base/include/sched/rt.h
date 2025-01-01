/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_RT_H
#define _LINUX_SCHED_RT_H

#include <sched.h>

static inline int rt_prio(int prio)
{
    if (unlikely(prio < MAX_RT_PRIO))
        return 1;
    return 0;
}

#endif /* _LINUX_SCHED_RT_H */

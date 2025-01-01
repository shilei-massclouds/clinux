/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_DL_H
#define _LINUX_SCHED_DL_H

#define MAX_DL_PRIO     0

static inline int dl_prio(int prio)
{
    if (unlikely(prio < MAX_DL_PRIO))
        return 1;
    return 0;
}

#endif /* _LINUX_SCHED_DL_H */

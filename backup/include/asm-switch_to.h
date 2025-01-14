/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __ASM_GENERIC_SWITCH_TO_H
#define __ASM_GENERIC_SWITCH_TO_H

#include <thread_info.h>

/*
 * Context switching is now performed out-of-line in switch_to.S
 */
extern struct task_struct *
__switch_to(struct task_struct *, struct task_struct *);

#define switch_to(prev, next, last)                 \
    do {                                \
        ((last) = __switch_to((prev), (next))); \
    } while (0)

#endif /* __ASM_GENERIC_SWITCH_TO_H */

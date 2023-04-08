/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PAYLOAD_H
#define _LINUX_PAYLOAD_H

#include <list.h>

struct payload {
    struct list_head lh;
    const char *name;   /* pointer to actual string */
    void *ptr;
    int size;
};

#endif /* _LINUX_PAYLOAD_H */

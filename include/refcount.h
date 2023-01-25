/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_REFCOUNT_H
#define _LINUX_REFCOUNT_H

#include <atomic.h>

typedef struct refcount_struct {
    atomic_t refs;
} refcount_t;

/**
 * refcount_set - set a refcount's value
 * @r: the refcount
 * @n: value to which the refcount will be set
 */
static inline void
refcount_set(refcount_t *r, int n)
{
    atomic_set(&r->refs, n);
}

static inline bool
refcount_sub_and_test(int i, refcount_t *r)
{
    int old = r->refs.counter;
    r->refs.counter -= i;

    if (old == i) {
        return true;
    }

    return false;
}

static inline bool
refcount_dec_and_test(refcount_t *r)
{
    return refcount_sub_and_test(1, r);
}

#endif /* _LINUX_REFCOUNT_H */

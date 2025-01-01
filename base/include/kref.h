/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _KREF_H_
#define _KREF_H_

#include <refcount.h>

struct kref {
    refcount_t refcount;
};

/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
static inline void
kref_init(struct kref *kref)
{
    refcount_set(&kref->refcount, 1);
}

static inline int
kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
    if (refcount_dec_and_test(&kref->refcount)) {
        release(kref);
        return 1;
    }
    return 0;
}

#endif /* _KREF_H_ */

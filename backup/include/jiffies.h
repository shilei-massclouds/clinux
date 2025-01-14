/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_JIFFIES_H
#define _LINUX_JIFFIES_H

#include <config.h>
#include <limits.h>

#define HZ              CONFIG_HZ   /* Internal kernel timer frequency */
#define MSEC_PER_SEC    1000L

/*
 * HZ is equal to or smaller than 1000, and 1000 is a nice round
 * multiple of HZ, divide with the factor between them, but round
 * upwards:
 */
static inline unsigned long _msecs_to_jiffies(const unsigned int m)
{
    return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
}

/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return LONG_MAX, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1)-1)

unsigned long __msecs_to_jiffies(const unsigned int m);

static __always_inline unsigned long
msecs_to_jiffies(const unsigned int m)
{
    if (__builtin_constant_p(m)) {
        if ((int)m < 0)
            return MAX_JIFFY_OFFSET;
        return _msecs_to_jiffies(m);
    } else {
        return __msecs_to_jiffies(m);
    }
}

#endif /* _LINUX_JIFFIES_H */

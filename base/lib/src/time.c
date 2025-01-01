// SPDX-License-Identifier: GPL-2.0

#include <export.h>
#include <jiffies.h>

unsigned long __msecs_to_jiffies(const unsigned int m)
{
    /*
     * Negative value, means infinite timeout:
     */
    if ((int)m < 0)
        return MAX_JIFFY_OFFSET;
    return _msecs_to_jiffies(m);
}
EXPORT_SYMBOL(__msecs_to_jiffies);


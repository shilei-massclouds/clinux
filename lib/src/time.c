// SPDX-License-Identifier: GPL-2.0
// From kernel/time/time.c and kernel/time/timer.c

#include <linux/types.h>
#include <linux/export.h>
#include <linux/jiffies.h>

// From init/calibrate.c
unsigned long lpj_fine;

unsigned long riscv_timebase;
EXPORT_SYMBOL_GPL(riscv_timebase);

/**
 * msecs_to_jiffies: - convert milliseconds to jiffies
 * @m:  time in milliseconds
 *
 * conversion is done as follows:
 *
 * - negative values mean 'infinite timeout' (MAX_JIFFY_OFFSET)
 *
 * - 'too large' values [that would result in larger than
 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
 *
 * - all other values are converted to jiffies by either multiplying
 *   the input value by a factor or dividing it with a factor and
 *   handling any 32-bit overflows.
 *   for the details see __msecs_to_jiffies()
 *
 * msecs_to_jiffies() checks for the passed in value being a constant
 * via __builtin_constant_p() allowing gcc to eliminate most of the
 * code, __msecs_to_jiffies() is called if the value passed does not
 * allow constant folding and the actual conversion must be done at
 * runtime.
 * the _msecs_to_jiffies helpers are the HZ dependent conversion
 * routines found in include/linux/jiffies.h
 */
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

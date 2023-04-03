/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H

#include <atomic.h>

#define aligned_byte_mask(n) ((1UL << 8*(n))-1)

/**
 * clear_bit_unlock_is_negative_byte - Clear a bit in memory and test if bottom
 *                                     byte is negative, for unlock.
 * @nr: the bit to clear
 * @addr: the address to start counting from
 *
 * This is a bit of a one-trick-pony for the filemap code, which clears
 * PG_locked and tests PG_waiters,
 */
#ifndef clear_bit_unlock_is_negative_byte
static inline bool
clear_bit_unlock_is_negative_byte(unsigned int nr,
                                  volatile unsigned long *p)
{
    long old;
    unsigned long mask = BIT_MASK(nr);

    p += BIT_WORD(nr);
    old = atomic_long_fetch_andnot_release(mask, (atomic_long_t *)p);
    return !!(old & BIT(7));
}
#define clear_bit_unlock_is_negative_byte \
    clear_bit_unlock_is_negative_byte
#endif

#endif /* _LINUX_BITOPS_H */

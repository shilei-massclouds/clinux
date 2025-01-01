// SPDX-License-Identifier: GPL-2.0+

#include <log2.h>
#include <export.h>
#include <printk.h>
#include <bitmap.h>

static unsigned long
_find_next_bit(const unsigned long *addr1,
               const unsigned long *addr2,
               unsigned long nbits,
               unsigned long start,
               unsigned long invert)
{
    unsigned long tmp, mask;

    if (unlikely(start >= nbits))
        return nbits;

    tmp = addr1[start / BITS_PER_LONG];
    if (addr2)
        tmp &= addr2[start / BITS_PER_LONG];
    tmp ^= invert;

    /* Handle 1st word. */
    mask = BITMAP_FIRST_WORD_MASK(start);

    tmp &= mask;

    start = round_down(start, BITS_PER_LONG);

    while (!tmp) {
        start += BITS_PER_LONG;
        if (start >= nbits)
            return nbits;

        tmp = addr1[start / BITS_PER_LONG];
        if (addr2)
            tmp &= addr2[start / BITS_PER_LONG];
        tmp ^= invert;
    }

    return min(start + __ffs(tmp), nbits);
}

unsigned long
find_next_zero_bit(const unsigned long *addr,
                   unsigned long size,
                   unsigned long offset)
{
    return _find_next_bit(addr, NULL, size, offset, ~0UL);
}
EXPORT_SYMBOL(find_next_zero_bit);

unsigned long
find_next_bit(const unsigned long *addr,
              unsigned long size,
              unsigned long offset)
{
    return _find_next_bit(addr, NULL, size, offset, 0UL);
}
EXPORT_SYMBOL(find_next_bit);
EXPORT_SYMBOL_ITF(find_next_bit, ilib);

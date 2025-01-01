/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_BITMAP_H
#define __LINUX_BITMAP_H

#include <bits.h>
#include <string.h>
#include <compiler_attributes.h>

#define BITMAP_MEM_ALIGNMENT 8
#define BITMAP_MEM_MASK (BITMAP_MEM_ALIGNMENT - 1)

#define BITMAP_FIRST_WORD_MASK(start) \
    (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) \
    (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

unsigned long
bitmap_find_next_zero_area_off(unsigned long *map,
                               unsigned long size,
                               unsigned long start,
                               unsigned int nr,
                               unsigned long align_mask,
                               unsigned long align_offset);

static inline unsigned long
bitmap_find_next_zero_area(unsigned long *map,
                           unsigned long size,
                           unsigned long start,
                           unsigned int nr,
                           unsigned long align_mask)
{
    return bitmap_find_next_zero_area_off(map, size, start, nr,
                                          align_mask, 0);
}

void __bitmap_set(unsigned long *map, unsigned int start, int len);

static __always_inline void
bitmap_set(unsigned long *map, unsigned int start, unsigned int nbits)
{
    if (__builtin_constant_p(nbits) && nbits == 1)
        __set_bit(start, map);
    else if (__builtin_constant_p(start & BITMAP_MEM_MASK) &&
         IS_ALIGNED(start, BITMAP_MEM_ALIGNMENT) &&
         __builtin_constant_p(nbits & BITMAP_MEM_MASK) &&
         IS_ALIGNED(nbits, BITMAP_MEM_ALIGNMENT))
        memset((char *)map + start / 8, 0xff, nbits / 8);
    else
        __bitmap_set(map, start, nbits);
}

#endif /* __LINUX_BITMAP_H */

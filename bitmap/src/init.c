// SPDX-License-Identifier: GPL-2.0+

#include <bitmap.h>
#include <export.h>
#include <kernel.h>
#include <printk.h>
#include <find_bit.h>

unsigned long
bitmap_find_next_zero_area_off(unsigned long *map,
                               unsigned long size,
                               unsigned long start,
                               unsigned int nr,
                               unsigned long align_mask,
                               unsigned long align_offset)
{
    unsigned long index, end, i;
again:
    index = find_next_zero_bit(map, size, start);

    /* Align allocation */
    index = __ALIGN_MASK(index + align_offset, align_mask) - align_offset;

    end = index + nr;
    if (end > size)
        return end;
    i = find_next_bit(map, end, index);
    if (i < end) {
        start = i + 1;
        goto again;
    }
    return index;
}
EXPORT_SYMBOL(bitmap_find_next_zero_area_off);

void __bitmap_set(unsigned long *map, unsigned int start, int len)
{
    unsigned long *p = map + BIT_WORD(start);
    const unsigned int size = start + len;
    int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

    while (len - bits_to_set >= 0) {
        *p |= mask_to_set;
        len -= bits_to_set;
        bits_to_set = BITS_PER_LONG;
        mask_to_set = ~0UL;
        p++;
    }
    if (len) {
        mask_to_set &= BITMAP_LAST_WORD_MASK(size);
        *p |= mask_to_set;
    }
}
EXPORT_SYMBOL(__bitmap_set);

int
init_module(void)
{
    printk("module[bitmap]: init begin ...\n");
    printk("module[bitmap]: init end!\n");
    return 0;
}

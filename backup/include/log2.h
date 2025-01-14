/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _LINUX_LOG2_H
#define _LINUX_LOG2_H

#include <bits.h>
#include <page.h>
#include <types.h>
#include <compiler_attributes.h>

static inline __attribute__((const))
bool is_power_of_2(unsigned long n)
{
    return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __always_inline unsigned long
__ffs(unsigned long word)
{
    int num = 0;

    if ((word & 0xffffffff) == 0) {
        num += 32;
        word >>= 32;
    }
    if ((word & 0xffff) == 0) {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0) {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0) {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0) {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
        num += 1;
    return num;
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static __always_inline int fls(unsigned int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

static __always_inline int __fls(unsigned long word)
{
    int num = BITS_PER_LONG - 1;

    if (!(word & (~0ul << 32))) {
        num -= 32;
        word <<= 32;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
        num -= 16;
        word <<= 16;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
        num -= 8;
        word <<= 8;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
        num -= 4;
        word <<= 4;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
        num -= 2;
        word <<= 2;
    }
    if (!(word & (~0ul << (BITS_PER_LONG-1))))
        num -= 1;
    return num;
}

static __always_inline int fls64(u64 x)
{
    if (x == 0)
        return 0;
    return __fls(x) + 1;
}

/*
 * non-constant log of base 2 calculators
 * - the arch may override these in asm/bitops.h if they can be implemented
 *   more efficiently than using fls() and fls64()
 * - the arch is not required to handle n==0 if implementing the fallback
 */
static inline __attribute__((const))
int __ilog2_u32(u32 n)
{
    return fls(n) - 1;
}

static inline __attribute__((const))
int __ilog2_u64(u64 n)
{
    return fls64(n) - 1;
}

/**
 * const_ilog2 - log base 2 of 32-bit or a 64-bit constant unsigned value
 * @n: parameter
 *
 * Use this where sparse expects a true constant expression, e.g. for array
 * indices.
 */
#define const_ilog2(n)              \
(                       \
    __builtin_constant_p(n) ? (     \
        (n) < 2 ? 0 :           \
        (n) & (1ULL << 63) ? 63 :   \
        (n) & (1ULL << 62) ? 62 :   \
        (n) & (1ULL << 61) ? 61 :   \
        (n) & (1ULL << 60) ? 60 :   \
        (n) & (1ULL << 59) ? 59 :   \
        (n) & (1ULL << 58) ? 58 :   \
        (n) & (1ULL << 57) ? 57 :   \
        (n) & (1ULL << 56) ? 56 :   \
        (n) & (1ULL << 55) ? 55 :   \
        (n) & (1ULL << 54) ? 54 :   \
        (n) & (1ULL << 53) ? 53 :   \
        (n) & (1ULL << 52) ? 52 :   \
        (n) & (1ULL << 51) ? 51 :   \
        (n) & (1ULL << 50) ? 50 :   \
        (n) & (1ULL << 49) ? 49 :   \
        (n) & (1ULL << 48) ? 48 :   \
        (n) & (1ULL << 47) ? 47 :   \
        (n) & (1ULL << 46) ? 46 :   \
        (n) & (1ULL << 45) ? 45 :   \
        (n) & (1ULL << 44) ? 44 :   \
        (n) & (1ULL << 43) ? 43 :   \
        (n) & (1ULL << 42) ? 42 :   \
        (n) & (1ULL << 41) ? 41 :   \
        (n) & (1ULL << 40) ? 40 :   \
        (n) & (1ULL << 39) ? 39 :   \
        (n) & (1ULL << 38) ? 38 :   \
        (n) & (1ULL << 37) ? 37 :   \
        (n) & (1ULL << 36) ? 36 :   \
        (n) & (1ULL << 35) ? 35 :   \
        (n) & (1ULL << 34) ? 34 :   \
        (n) & (1ULL << 33) ? 33 :   \
        (n) & (1ULL << 32) ? 32 :   \
        (n) & (1ULL << 31) ? 31 :   \
        (n) & (1ULL << 30) ? 30 :   \
        (n) & (1ULL << 29) ? 29 :   \
        (n) & (1ULL << 28) ? 28 :   \
        (n) & (1ULL << 27) ? 27 :   \
        (n) & (1ULL << 26) ? 26 :   \
        (n) & (1ULL << 25) ? 25 :   \
        (n) & (1ULL << 24) ? 24 :   \
        (n) & (1ULL << 23) ? 23 :   \
        (n) & (1ULL << 22) ? 22 :   \
        (n) & (1ULL << 21) ? 21 :   \
        (n) & (1ULL << 20) ? 20 :   \
        (n) & (1ULL << 19) ? 19 :   \
        (n) & (1ULL << 18) ? 18 :   \
        (n) & (1ULL << 17) ? 17 :   \
        (n) & (1ULL << 16) ? 16 :   \
        (n) & (1ULL << 15) ? 15 :   \
        (n) & (1ULL << 14) ? 14 :   \
        (n) & (1ULL << 13) ? 13 :   \
        (n) & (1ULL << 12) ? 12 :   \
        (n) & (1ULL << 11) ? 11 :   \
        (n) & (1ULL << 10) ? 10 :   \
        (n) & (1ULL <<  9) ?  9 :   \
        (n) & (1ULL <<  8) ?  8 :   \
        (n) & (1ULL <<  7) ?  7 :   \
        (n) & (1ULL <<  6) ?  6 :   \
        (n) & (1ULL <<  5) ?  5 :   \
        (n) & (1ULL <<  4) ?  4 :   \
        (n) & (1ULL <<  3) ?  3 :   \
        (n) & (1ULL <<  2) ?  2 :   \
        1) :                \
    -1)

/**
 * ilog2 - log base 2 of 32-bit or a 64-bit unsigned value
 * @n: parameter
 *
 * constant-capable log of base 2 calculation
 * - this can be used to initialise global variables from constant data, hence
 * the massive ternary operator construction
 *
 * selects the appropriately-sized optimised version depending on sizeof(n)
 */
#define ilog2(n) \
( \
    __builtin_constant_p(n) ?   \
    const_ilog2(n) :        \
    (sizeof(n) <= 4) ?      \
    __ilog2_u32(n) :        \
    __ilog2_u64(n)          \
)

static inline __attribute_const__
int __order_base_2(unsigned long n)
{
    return n > 1 ? ilog2(n - 1) + 1 : 0;
}

/**
 * order_base_2 - calculate the (rounded up) base 2 order of the argument
 * @n: parameter
 *
 * The first few values calculated by this routine:
 *  ob2(0) = 0
 *  ob2(1) = 0
 *  ob2(2) = 1
 *  ob2(3) = 2
 *  ob2(4) = 2
 *  ob2(5) = 3
 *  ... and so on.
 */
#define order_base_2(n)             \
(                       \
    __builtin_constant_p(n) ? (     \
        ((n) == 0 || (n) == 1) ? 0 :    \
        ilog2((n) - 1) + 1) :       \
    __order_base_2(n)           \
)

static inline unsigned int
fls_long(unsigned long l)
{
    if (sizeof(l) == 4)
        return fls(l);
    return fls64(l);
}

/**
 * get_count_order_long - get order after rounding @l up to power of 2
 * @l: parameter
 *
 * it is same as get_count_order() but with long type parameter
 */
static inline int
get_count_order_long(unsigned long l)
{
    if (l == 0UL)
        return -1;
    else if (l & (l - 1UL))
        return (int)fls_long(l);
    else
        return (int)fls_long(l) - 1;
}

/**
 * get_order - Determine the allocation order of a memory size
 * @size: The size for which to get the order
 *
 * Determine the allocation order of a particular sized block of memory.  This
 * is on a logarithmic scale, where:
 *
 *  0 -> 2^0 * PAGE_SIZE and below
 *  1 -> 2^1 * PAGE_SIZE to 2^0 * PAGE_SIZE + 1
 *  2 -> 2^2 * PAGE_SIZE to 2^1 * PAGE_SIZE + 1
 *  3 -> 2^3 * PAGE_SIZE to 2^2 * PAGE_SIZE + 1
 *  4 -> 2^4 * PAGE_SIZE to 2^3 * PAGE_SIZE + 1
 *  ...
 *
 * The order returned is used to find the smallest allocation granule required
 * to hold an object of the specified size.
 *
 * The result is undefined if the size is 0.
 */
static inline __attribute_const__ int
get_order(unsigned long size)
{
    if (__builtin_constant_p(size)) {
        if (!size)
            return BITS_PER_LONG - PAGE_SHIFT;

        if (size < (1UL << PAGE_SHIFT))
            return 0;

        return ilog2((size) - 1) - PAGE_SHIFT + 1;
    }

    size--;
    size >>= PAGE_SHIFT;
    return fls64(size);
}

/**
 * __roundup_pow_of_two() - round up to nearest power of two
 * @n: value to round up
 */
static inline __attribute__((const))
unsigned long __roundup_pow_of_two(unsigned long n)
{
    return 1UL << fls_long(n - 1);
}

/**
 * __rounddown_pow_of_two() - round down to nearest power of two
 * @n: value to round down
 */
static inline __attribute__((const))
unsigned long __rounddown_pow_of_two(unsigned long n)
{
    return 1UL << (fls_long(n) - 1);
}

/**
 * roundup_pow_of_two - round the given value up to nearest power of two
 * @n: parameter
 *
 * round the given value up to the nearest power of two
 * - the result is undefined when n == 0
 * - this can be used to initialise global variables from constant data
 */
#define roundup_pow_of_two(n)   \
(                               \
    __builtin_constant_p(n) ? ( \
        ((n) == 1) ? 1 :        \
        (1UL << (ilog2((n) - 1) + 1))   \
    ) : \
    __roundup_pow_of_two(n)     \
)

/**
 * rounddown_pow_of_two - round the given value down to nearest power of two
 * @n: parameter
 *
 * round the given value down to the nearest power of two
 * - the result is undefined when n == 0
 * - this can be used to initialise global variables from constant data
 */
#define rounddown_pow_of_two(n)         \
(                       \
    __builtin_constant_p(n) ? (     \
        (1UL << ilog2(n))) :        \
    __rounddown_pow_of_two(n)       \
)


#endif /* _LINUX_LOG2_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _GENERIC_BITS_H_
#define _GENERIC_BITS_H_

#include <compiler_attributes.h>

# define do_div(n,base) ({                  \
    uint32_t __base = (base);               \
    uint32_t __rem;                     \
    __rem = ((uint64_t)(n)) % __base;           \
    (n) = ((uint64_t)(n)) / __base;             \
    __rem;                          \
 })

#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP

#define DIV_ROUND_DOWN_ULL(ll, d) \
    ({ unsigned long long _tmp = (ll); do_div(_tmp, d); _tmp; })

#define DIV_ROUND_UP_ULL(ll, d) \
    DIV_ROUND_DOWN_ULL((unsigned long long)(ll) + (d) - 1, (d))

#define BITS_PER_LONG       64
#define BITS_PER_LONG_LONG  64

#define BIT(nr)             (1UL << (nr))
#define BIT_ULL(nr)         (1ULL << (nr))
#define BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)    (1ULL << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)    ((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE       8

#define BITS_PER_TYPE(type) (sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

#define __AMO(op)   "amo" #op ".d"

#define __test_and_op_bit_ord(op, mod, nr, addr, ord)   \
({                                                      \
    unsigned long __res, __mask;                        \
    __mask = BIT_MASK(nr);                              \
    __asm__ __volatile__ (                              \
        __AMO(op) #ord " %0, %2, %1"                    \
        : "=r" (__res), "+A" (addr[BIT_WORD(nr)])       \
        : "r" (mod(__mask))                             \
        : "memory");                                    \
    ((__res & __mask) != 0);                            \
})

#define __op_bit_ord(op, mod, nr, addr, ord) \
    __asm__ __volatile__ (                   \
        __AMO(op) #ord " zero, %1, %0"       \
        : "+A" (addr[BIT_WORD(nr)])          \
        : "r" (mod(BIT_MASK(nr)))            \
        : "memory");

#define __test_and_op_bit(op, mod, nr, addr) \
    __test_and_op_bit_ord(op, mod, nr, addr, .aqrl)

#define __op_bit(op, mod, nr, addr) \
    __op_bit_ord(op, mod, nr, addr, )

/* Bitmask modifiers */
#define __NOP(x)    (x)
#define __NOT(x)    (~(x))

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Unlike set_bit(), this function is non-atomic and may be reordered.
 * If it's called on the same region of memory simultaneously, the effect
 * may be that only one operation succeeds.
 */
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

    *p |= mask;
}

static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

    *p &= ~mask;
}

/**
 * set_bit - Atomically set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 *
 * Note that @nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void set_bit(int nr, volatile unsigned long *addr)
{
    __op_bit(or, __NOP, nr, addr);
}

/**
 * clear_bit - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * Note: there are no guarantees that this function will not be reordered
 * on non x86 architectures, so if you are writing portable code,
 * make sure not to rely on its reordering guarantees.
 */
static inline void clear_bit(int nr, volatile unsigned long *addr)
{
    __op_bit(and, __NOT, nr, addr);
}

/**
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation may be reordered on other architectures than x86.
 */
static inline int test_and_set_bit(int nr, volatile unsigned long *addr)
{
    return __test_and_op_bit(or, __NOP, nr, addr);
}

static inline int
test_bit(int nr, const volatile unsigned long *addr)
{
    return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

/**
 * test_and_set_bit_lock - Set a bit and return its old value, for lock
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and provides acquire barrier semantics.
 * It can be used to implement bit locks.
 */
static inline int
test_and_set_bit_lock(unsigned long nr, volatile unsigned long *addr)
{
    return __test_and_op_bit_ord(or, __NOP, nr, addr, .aq);
}

static inline void
clear_bit_unlock(unsigned long nr, volatile unsigned long *addr)
{
    __op_bit_ord(and, __NOT, nr, addr, .rl);
}

static inline int
test_and_clear_bit(int nr, volatile unsigned long *addr)
{
    return __test_and_op_bit(and, __NOT, nr, addr);
}

#endif /* _GENERIC_BITS_H_ */

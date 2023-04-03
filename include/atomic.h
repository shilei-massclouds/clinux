// SPDX-License-Identifier: GPL-2.0
#ifndef _ASM_GENERIC_ATOMIC_LONG_H
#define _ASM_GENERIC_ATOMIC_LONG_H

#include <bug.h>
#include <fence.h>
#include <types.h>
#include <compiler_attributes.h>

#define __atomic_acquire_fence()                    \
    __asm__ __volatile__(RISCV_ACQUIRE_BARRIER "" ::: "memory")

#define __atomic_release_fence()                    \
    __asm__ __volatile__(RISCV_RELEASE_BARRIER "" ::: "memory");

/*
 * Use __READ_ONCE() instead of READ_ONCE() if you do not require any
 * atomicity. Note that this may result in tears!
 */
#ifndef __READ_ONCE
#define __READ_ONCE(x)  (*(const volatile __unqual_scalar_typeof(x) *)&(x))
#endif

#define READ_ONCE(x)    \
({                      \
    __READ_ONCE(x);     \
})

#define __WRITE_ONCE(x, val)                        \
do {                                    \
    *(volatile typeof(x) *)&(x) = (val);                \
} while (0)

#define WRITE_ONCE(x, val)  \
do {                        \
    __WRITE_ONCE(x, val);   \
} while (0)

#define atomic_read(v)  READ_ONCE((v)->counter)
#define atomic_set(v, i) WRITE_ONCE(((v)->counter), (i))

typedef struct {
    int counter;
} atomic_t;

typedef struct {
    s64 counter;
} atomic64_t;

typedef atomic64_t atomic_long_t;

static __always_inline s64
atomic64_read(const atomic64_t *v)
{
    return READ_ONCE(v->counter);
}

static __always_inline void
atomic64_set(atomic64_t *v, s64 i)
{
    WRITE_ONCE(v->counter, i);
}

/*
 * First, the atomic ops that have no ordering constraints and therefor don't
 * have the AQ or RL bits set.  These don't return anything, so there's only
 * one version to worry about.
 */
#define ATOMIC_OP(op, asm_op, I, asm_type, c_type, prefix)  \
static __always_inline                                      \
void atomic##prefix##_##op(c_type i, atomic##prefix##_t *v) \
{                                                           \
    __asm__ __volatile__ (                                  \
        "   amo" #asm_op "." #asm_type " zero, %1, %0"      \
        : "+A" (v->counter)                                 \
        : "r" (I)                                           \
        : "memory");                                        \
}

#define ATOMIC_OPS(op, asm_op, I)               \
        ATOMIC_OP (op, asm_op, I, w, int,   )   \
        ATOMIC_OP (op, asm_op, I, d, s64, 64)

ATOMIC_OPS(add, add,  i)
ATOMIC_OPS(sub, add, -i)
ATOMIC_OPS(and, and,  i)
ATOMIC_OPS( or,  or,  i)
ATOMIC_OPS(xor, xor,  i)

#undef ATOMIC_OP
#undef ATOMIC_OPS

/*
 * Atomic ops that have ordered, relaxed, acquire, and release variants.
 * There's two flavors of these: the arithmatic ops have both fetch and return
 * versions, while the logical ops only have fetch versions.
 */
#define ATOMIC_FETCH_OP(op, asm_op, I, asm_type, c_type, prefix)    \
static __always_inline                          \
c_type atomic##prefix##_fetch_##op##_relaxed(c_type i,          \
                         atomic##prefix##_t *v) \
{                                   \
    register c_type ret;                        \
    __asm__ __volatile__ (                      \
        "   amo" #asm_op "." #asm_type " %1, %2, %0"    \
        : "+A" (v->counter), "=r" (ret)             \
        : "r" (I)                       \
        : "memory");                        \
    return ret;                         \
}                                   \
static __always_inline                          \
c_type atomic##prefix##_fetch_##op(c_type i, atomic##prefix##_t *v) \
{                                   \
    register c_type ret;                        \
    __asm__ __volatile__ (                      \
        "   amo" #asm_op "." #asm_type ".aqrl  %1, %2, %0"  \
        : "+A" (v->counter), "=r" (ret)             \
        : "r" (I)                       \
        : "memory");                        \
    return ret;                         \
}

#define ATOMIC_OP_RETURN(op, asm_op, c_op, I, asm_type, c_type, prefix) \
static __always_inline                          \
c_type atomic##prefix##_##op##_return_relaxed(c_type i,         \
                          atomic##prefix##_t *v)    \
{                                   \
        return atomic##prefix##_fetch_##op##_relaxed(i, v) c_op I;  \
}                                   \
static __always_inline                          \
c_type atomic##prefix##_##op##_return(c_type i, atomic##prefix##_t *v)  \
{                                   \
        return atomic##prefix##_fetch_##op(i, v) c_op I;        \
}

#define ATOMIC_OPS(op, asm_op, c_op, I)                 \
        ATOMIC_FETCH_OP( op, asm_op,       I, w, int,   )       \
        ATOMIC_OP_RETURN(op, asm_op, c_op, I, w, int,   )       \
        ATOMIC_FETCH_OP( op, asm_op,       I, d, s64, 64)       \
        ATOMIC_OP_RETURN(op, asm_op, c_op, I, d, s64, 64)

ATOMIC_OPS(add, add, +,  i)
ATOMIC_OPS(sub, add, +, -i)

#define atomic_add_return_relaxed   atomic_add_return_relaxed
#define atomic_sub_return_relaxed   atomic_sub_return_relaxed
#define atomic_add_return       atomic_add_return
#define atomic_sub_return       atomic_sub_return

#define atomic_fetch_add_relaxed    atomic_fetch_add_relaxed
#define atomic_fetch_sub_relaxed    atomic_fetch_sub_relaxed
#define atomic_fetch_add        atomic_fetch_add
#define atomic_fetch_sub        atomic_fetch_sub

#define atomic64_add_return_relaxed atomic64_add_return_relaxed
#define atomic64_sub_return_relaxed atomic64_sub_return_relaxed
#define atomic64_add_return     atomic64_add_return
#define atomic64_sub_return     atomic64_sub_return

#define atomic64_fetch_add_relaxed  atomic64_fetch_add_relaxed
#define atomic64_fetch_sub_relaxed  atomic64_fetch_sub_relaxed
#define atomic64_fetch_add      atomic64_fetch_add
#define atomic64_fetch_sub      atomic64_fetch_sub

#undef ATOMIC_OPS

#define ATOMIC_OPS(op, asm_op, I)                   \
        ATOMIC_FETCH_OP(op, asm_op, I, w, int,   )          \
        ATOMIC_FETCH_OP(op, asm_op, I, d, s64, 64)

ATOMIC_OPS(and, and, i)
ATOMIC_OPS( or,  or, i)
ATOMIC_OPS(xor, xor, i)

#define atomic_fetch_and_relaxed    atomic_fetch_and_relaxed
#define atomic_fetch_or_relaxed     atomic_fetch_or_relaxed
#define atomic_fetch_xor_relaxed    atomic_fetch_xor_relaxed
#define atomic_fetch_and        atomic_fetch_and
#define atomic_fetch_or         atomic_fetch_or
#define atomic_fetch_xor        atomic_fetch_xor

#define atomic64_fetch_and_relaxed  atomic64_fetch_and_relaxed
#define atomic64_fetch_or_relaxed   atomic64_fetch_or_relaxed
#define atomic64_fetch_xor_relaxed  atomic64_fetch_xor_relaxed
#define atomic64_fetch_and      atomic64_fetch_and
#define atomic64_fetch_or       atomic64_fetch_or
#define atomic64_fetch_xor      atomic64_fetch_xor

#undef ATOMIC_OPS

#undef ATOMIC_FETCH_OP
#undef ATOMIC_OP_RETURN

static __always_inline void
atomic_long_set(atomic_long_t *v, long i)
{
    atomic64_set(v, i);
}

static __always_inline void
atomic_long_add(long i, atomic_long_t *v)
{
    atomic64_add(i, v);
}

static __always_inline int
atomic_dec_return(atomic_t *v)
{
    return atomic_sub_return(1, v);
}

#define arch_atomic_inc atomic_inc

#ifndef atomic_inc
static __always_inline void
atomic_inc(atomic_t *v)
{
    atomic_add(1, v);
}
#define atomic_inc atomic_inc
#endif

#define arch_atomic_dec atomic_dec

#ifndef atomic_dec
static __always_inline void
atomic_dec(atomic_t *v)
{
    atomic_sub(1, v);
}
#define atomic_dec atomic_dec
#endif

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static __always_inline bool
atomic_dec_and_test(atomic_t *v)
{
    return atomic_dec_return(v) == 0;
}

static __always_inline long
atomic_long_read(const atomic_long_t *v)
{
    return atomic64_read(v);
}

#ifndef atomic64_fetch_and_release
static __always_inline s64
atomic64_fetch_and_release(s64 i, atomic64_t *v)
{
    __atomic_release_fence();
    return atomic64_fetch_and_relaxed(i, v);
}
#define atomic64_fetch_and_release atomic64_fetch_and_release
#endif

#ifndef atomic64_fetch_andnot_release
static __always_inline s64
atomic64_fetch_andnot_release(s64 i, atomic64_t *v)
{
    return atomic64_fetch_and_release(~i, v);
}
#define atomic64_fetch_andnot_release atomic64_fetch_andnot_release
#endif

static __always_inline long
atomic_long_fetch_andnot_release(long i, atomic_long_t *v)
{
    return atomic64_fetch_andnot_release(i, v);
}

#endif /* _ASM_GENERIC_ATOMIC_LONG_H */

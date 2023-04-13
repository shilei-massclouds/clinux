/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#define swab32(x) ((u32)(                    \
    (((u32)(x) & (u32)0x000000ffUL) << 24) | \
    (((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
    (((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
    (((u32)(x) & (u32)0xff000000UL) >> 24)))

#ifndef __ASSEMBLY__

#include <bits.h>

#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))

extern const unsigned char _ctype[];

#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
static inline int isdigit(int c)
{
    return '0' <= c && c <= '9';
}
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
/* Note: isspace() must return false for %NUL-terminator */
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)

#define NULL ((void *)0)

#define SIZE_MAX        (~(size_t)0)
#define PHYS_ADDR_MAX   (~(phys_addr_t)0)
#define INT_MAX         ((int)(~0U >> 1))
#define INT_MIN         (-INT_MAX - 1)
#define ULLONG_MAX      (~0ULL)

#define ATOMIC_INIT(i) { (i) }

#define min(a, b)   ((a < b) ? a : b)
#define max(a, b)   ((a > b) ? a : b)

#define min3(x, y, z) min((typeof(x))min(x, y), z)

#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define __typecheck(x, y) \
        (!!(sizeof((typeof(x) *)1 == (typeof(y) *)1)))

#define __is_constexpr(x) \
    (sizeof(int) == sizeof(*(8 ? ((void *)((long)(x) * 0l)) : (int *)8)))

#define __no_side_effects(x, y) \
        (__is_constexpr(x) && __is_constexpr(y))

#define __safe_cmp(x, y) \
        (__typecheck(x, y) && __no_side_effects(x, y))

#define __cmp(x, y, op) ((x) op (y) ? (x) : (y))

#define __cmp_once(x, y, unique_x, unique_y, op) ({ \
        typeof(x) unique_x = (x);       \
        typeof(y) unique_y = (y);       \
        __cmp(unique_x, unique_y, op); })

#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

#define __UNIQUE_ID(prefix) \
    __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)

#define __careful_cmp(x, y, op) \
    __builtin_choose_expr(__safe_cmp(x, y), \
        __cmp(x, y, op), \
        __cmp_once(x, y, __UNIQUE_ID(__x), __UNIQUE_ID(__y), op))

/**
 * min_t - return minimum of two values, using the specified type
 * @type: data type to use
 * @x: first value
 * @y: second value
 */
#define min_t(type, x, y)   __careful_cmp((type)(x), (type)(y), <)

/**
 * max_t - return maximum of two values, using the specified type
 * @type: data type to use
 * @x: first value
 * @y: second value
 */
#define max_t(type, x, y)   __careful_cmp((type)(x), (type)(y), >)

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * @type to make all the comparisons.
 */
#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)

typedef unsigned long   uintptr_t;

/*
 * This type is the placeholder for a hardware interrupt number. It has to be
 * big enough to enclose whatever representation is used by a given platform.
 */
typedef unsigned long irq_hw_number_t;

typedef _Bool bool;

enum {
    false   = 0,
    true    = 1
};

typedef int     s32;
typedef long    s64;

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef unsigned long   u64;

typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef u8  __u8;
typedef u16 __u16;
typedef u32 __u32;
typedef u64 __u64;

typedef s32 __s32;
typedef s64 __s64;

typedef u64 phys_addr_t;

typedef long                __kernel_long_t;
typedef unsigned long       __kernel_ulong_t;
typedef __kernel_ulong_t    __kernel_size_t;
typedef __kernel_long_t     __kernel_ssize_t;
typedef __kernel_long_t     __kernel_off_t;

typedef long long __kernel_time64_t;

typedef __kernel_long_t     __kernel_ptrdiff_t;
typedef __kernel_ptrdiff_t  ptrdiff_t;

typedef long long           __kernel_loff_t;
typedef __kernel_loff_t     loff_t;

typedef int __kernel_pid_t;
typedef __kernel_pid_t  pid_t;


#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t     size_t;
#endif

typedef __kernel_off_t      off_t;

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __kernel_ssize_t    ssize_t;
#endif

typedef unsigned int    __kernel_uid32_t;
typedef unsigned int    __kernel_gid32_t;

typedef __kernel_uid32_t    uid_t;
typedef __kernel_gid32_t    gid_t;

typedef int __kernel_clockid_t;
typedef __kernel_clockid_t  clockid_t;

typedef u64 dma_addr_t;

typedef u64 sector_t;
typedef u64 blkcnt_t;

typedef u32 __kernel_dev_t;
typedef __kernel_dev_t dev_t;

typedef __kernel_ulong_t    __kernel_ino_t;
typedef __kernel_ino_t      ino_t;

typedef unsigned short  umode_t;
typedef unsigned int    fmode_t;

typedef phys_addr_t resource_size_t;

typedef unsigned int gfp_t;
typedef unsigned int slab_flags_t;

/*
 * The type of an index into the pagecache.
 */
#define pgoff_t unsigned long

static inline u32 __swab32p(const u32 *p)
{
    return swab32(*p);
}

#define DECLARE_BITMAP(name, bits) \
    unsigned long name[BITS_TO_LONGS(bits)]

#define NR_CPUS 1

/* Don't assign or return these: may not be this big! */
typedef struct cpumask { DECLARE_BITMAP(bits, NR_CPUS); } cpumask_t;

/*
 * Fast implementation of tolower() for internal usage. Do not use in your
 * code.
 */
static inline char _tolower(const char c)
{
    return c | 0x20;
}

#endif /*  __ASSEMBLY__ */

#define be32_to_cpu(x)  swab32((u32)(x))
#define be32_to_cpup(x) __swab32p((u32 *)(x))
#define cpu_to_be32(x)  ((u32)swab32((x)))

#define _ALIGN_MASK(x, mask)    (((x) + (mask)) & ~(mask))
#define _ALIGN(x, a)            _ALIGN_MASK((x), (typeof(x))(a) - 1)
#define _ALIGN_DOWN(x, a)       _ALIGN((x) - ((a) - 1), (a))
#define PTR_ALIGN(p, a)         ((typeof(p))_ALIGN((unsigned long)(p), (a)))
#define PTR_ALIGN_DOWN(p, a)    ((typeof(p))_ALIGN_DOWN((unsigned long)(p), (a)))

#define IS_ALIGNED(x, a)    (((x) & ((typeof(x))(a) - 1)) == 0)

#endif /* _LINUX_TYPES_H */

/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef __UK_SYSCALL_H__
#define __UK_SYSCALL_H__

#include <stdarg.h>

typedef long uk_syscall_arg_t;

/* Raw system call, returns negative codes on errors */
long uk_syscall_r(long nr, ...);
long uk_vsyscall_r(long nr, va_list arg);
long uk_syscall6_r(long nr,
                   long arg1, long arg2, long arg3,
                   long arg4, long arg5, long arg6);

/* System call, returns -1 and sets errno on errors */
long uk_syscall(long nr, ...);
long uk_vsyscall(long nr, va_list arg);
long uk_syscall6(long nr,
                 long arg1, long arg2, long arg3,
                 long arg4, long arg5, long arg6);

#ifndef UK_CONCAT
#define __UK_CONCAT_X(a, b) a##b
#define UK_CONCAT(a, b) __UK_CONCAT_X(a, b)
#endif

#ifndef UK_NARGS
#define __UK_NARGS_X(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, \
             t, u, v, w, x, y, z, count, ...) count
#define UK_NARGS(...) \
    __UK_NARGS_X(, ##__VA_ARGS__, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
             15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif /* UK_NARGS */

/*
 * Use this variant instead of `uk_syscall_r()`
 * whenever the system call number is a constant.
 * This macro maps the function call directly to the target
 * handler instead of doing a look-up at runtime
 */
#define uk_syscall_r_static0(syscall_nr, ...) \
    UK_CONCAT(uk_syscall_r0_fn, syscall_nr)()
#define uk_syscall_r_static1(syscall_nr, a) \
    UK_CONCAT(uk_syscall_r1_fn, syscall_nr)(a)
#define uk_syscall_r_static2(syscall_nr, a, b) \
    UK_CONCAT(uk_syscall_r2_fn, syscall_nr)(a, b)
#define uk_syscall_r_static3(syscall_nr, a, b, c) \
    UK_CONCAT(uk_syscall_r3_fn, syscall_nr)(a, b, c)
#define uk_syscall_r_static4(syscall_nr, a, b, c, d) \
    UK_CONCAT(uk_syscall_r4_fn, syscall_nr)(a, b, c, d)
#define uk_syscall_r_static5(syscall_nr, a, b, c, d, e) \
    UK_CONCAT(uk_syscall_r5_fn, syscall_nr)(a, b, c, d, e)
#define uk_syscall_r_static6(syscall_nr, a, b, c, d, e, f) \
    UK_CONCAT(uk_syscall_r6_fn, syscall_nr)(a, b, c, d, e, f)

#define uk_syscall_r_static(syscall_nr, ...)            \
    UK_CONCAT(uk_syscall_r_static,              \
          UK_NARGS(__VA_ARGS__))(syscall_nr, __VA_ARGS__)

#endif /* __UK_SYSCALL_H__ */

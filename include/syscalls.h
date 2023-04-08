/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _LINUX_SYSCALLS_H
#define _LINUX_SYSCALLS_H

#include <types.h>
#include <utsname.h>
#include <futex.h>
#include <time_types.h>

/* Are two types/vars the same type (ignoring qualifiers)? */
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

/*
 * __MAP - apply a macro to syscall arguments
 * __MAP(n, m, t1, a1, t2, a2, ..., tn, an) will expand to
 *    m(t1, a1), m(t2, a2), ..., m(tn, an)
 * The first argument must be equal to the amount of type/name
 * pairs given.  Note that this list of pairs (i.e. the arguments
 * of __MAP starting at the third one) is in the same format as
 * for SYSCALL_DEFINE<n>/COMPAT_SYSCALL_DEFINE<n>
 */
#define __MAP0(m,...)
#define __MAP1(m,t,a,...) m(t,a)
#define __MAP2(m,t,a,...) m(t,a), __MAP1(m,__VA_ARGS__)
#define __MAP3(m,t,a,...) m(t,a), __MAP2(m,__VA_ARGS__)
#define __MAP4(m,t,a,...) m(t,a), __MAP3(m,__VA_ARGS__)
#define __MAP5(m,t,a,...) m(t,a), __MAP4(m,__VA_ARGS__)
#define __MAP6(m,t,a,...) m(t,a), __MAP5(m,__VA_ARGS__)
#define __MAP(n,...) __MAP##n(__VA_ARGS__)

#define __SC_DECL(t, a) t a
#define __TYPE_AS(t, v) __same_type(0, v)
#define __TYPE_IS_LL(t) (__TYPE_AS(t, 0LL) || __TYPE_AS(t, 0ULL))
#define __SC_LONG(t, a) __typeof(__builtin_choose_expr(__TYPE_IS_LL(t), 0LL, 0L)) a
#define __SC_CAST(t, a) (t) a
#define __SC_TEST(t, a) (void)(!__TYPE_IS_LL(t) && sizeof(t) > sizeof(long))

#define __SYSCALL_DEFINEx(x, name, ...)             \
    static inline long __do_sys##name(__MAP(x,__SC_DECL,__VA_ARGS__));\
    long sys##name(__MAP(x,__SC_DECL,__VA_ARGS__))  \
    {                               \
        long ret = __do_sys##name(__MAP(x,__SC_CAST,__VA_ARGS__));\
        __MAP(x,__SC_TEST,__VA_ARGS__);             \
        return ret;                     \
    }                               \
    static inline long __do_sys##name(__MAP(x,__SC_DECL,__VA_ARGS__))

#define SYSCALL_DEFINE1(name, ...) SYSCALL_DEFINEx(1, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE2(name, ...) SYSCALL_DEFINEx(2, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE3(name, ...) SYSCALL_DEFINEx(3, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE4(name, ...) SYSCALL_DEFINEx(4, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE5(name, ...) SYSCALL_DEFINEx(5, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE6(name, ...) SYSCALL_DEFINEx(6, _##name, __VA_ARGS__)

#define SYSCALL_DEFINEx(x, sname, ...) \
    __SYSCALL_DEFINEx(x, sname, __VA_ARGS__)

long sys_ni_syscall(void);

typedef long (*do_sys_open_t)(int dfd, const char *filename, int flags,
                              umode_t mode);
extern do_sys_open_t do_sys_open;

long sys_openat(int dfd, const char *filename, int flags, umode_t mode);

typedef long (*do_faccessat_t)(int dfd, const char *filename,
                               int mode, int flags);
extern do_faccessat_t do_faccessat;

long sys_faccessat(int dfd, const char *filename, int mode);

typedef long (*do_sys_brk_t)(unsigned long brk);
extern do_sys_brk_t do_sys_brk;

typedef long (*do_sys_readlinkat_t)(int dfd, const char *pathname,
                                    char *buf, int bufsiz);
extern do_sys_readlinkat_t do_sys_readlinkat;

long sys_readlinkat(int dfd, const char *pathname, char *buf, int bufsiz);

long sys_newuname(struct new_utsname *name);

typedef long (*do_sys_newuname_t)(struct new_utsname *name);
extern do_sys_newuname_t do_sys_newuname;

long sys_brk(unsigned long brk);

long sys_mprotect(unsigned long start, size_t len, unsigned long prot);

typedef long (*do_sys_mprotect_t)(unsigned long start, size_t len,
                                  unsigned long prot);

extern do_sys_mprotect_t do_sys_mprotect;

typedef long (*do_sys_mount_t)(char *dev_name, char *dir_name,
                               char *type, unsigned long flags,
                               void *data);

extern do_sys_mount_t do_sys_mount;

long sys_mount(char *dev_name, char *dir_name,
               char *type, unsigned long flags, void *data);

typedef long (*ksys_write_t)(unsigned int fd,
                             const char *buf, size_t count);

extern ksys_write_t ksys_write;

long sys_write(unsigned int fd, const char *buf, size_t count);

typedef void (*do_group_exit_t)(int exit_code);
extern do_group_exit_t do_group_exit;

long sys_exit_group(int error_code);

typedef long (*do_set_tid_address_t)(int *tidptr);
extern do_set_tid_address_t do_set_tid_address;

long sys_set_tid_address(int *tidptr);

long sys_set_robust_list(struct robust_list_head *head, size_t len);

long sys_prlimit64(pid_t pid, unsigned int resource,
                   const struct rlimit64 *new_rlim,
                   struct rlimit64 *old_rlim);

long sys_getrandom(char *buf, size_t count, unsigned int flags);

long sys_clock_gettime(clockid_t which_clock,
                       struct __kernel_timespec *tp);

long sys_mmap(unsigned long addr, unsigned long len,
              unsigned long prot, unsigned long flags,
              unsigned long fd, off_t offset);

#endif /* _LINUX_SYSCALLS_H */

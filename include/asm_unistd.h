/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

/* fs/ioctl.c */
#define __NR_ioctl 29
__SYSCALL(__NR_ioctl, sys_ioctl)

/* fs/namespace.c */
#define __NR_mount 40
__SYSCALL(__NR_mount, sys_mount)

/* fs/open.c */
#define __NR_faccessat 48
__SYSCALL(__NR_faccessat, sys_faccessat)

#define __NR_openat 56
__SYSCALL(__NR_openat, sys_openat)

/* fs/read_write.c */
#define __NR_write 64
__SYSCALL(__NR_write, sys_write)

#define __NR_writev 66
__SYSCALL(__NR_writev, sys_writev)

/* fs/stat.c */
#define __NR_readlinkat 78
__SYSCALL(__NR_readlinkat, sys_readlinkat)

/* kernel/exit.c */
#define __NR_exit_group 94
__SYSCALL(__NR_exit_group, sys_exit_group)

#define __NR_set_tid_address 96
__SYSCALL(__NR_set_tid_address, sys_set_tid_address)

#define __NR_set_robust_list 99
__SYSCALL(__NR_set_robust_list, sys_set_robust_list)

#define __NR_clock_gettime 113
__SYSCALL(__NR_clock_gettime, sys_clock_gettime)

/* sys/sys.c */
#define __NR_uname 160
__SYSCALL(__NR_uname, sys_newuname)

/* mm/nommu.c, also with MMU */
#define __NR_brk 214
__SYSCALL(__NR_brk, sys_brk)

#define __NR_munmap 215
__SYSCALL(__NR_munmap, sys_munmap)

#define __NR_mmap 222
__SYSCALL(__NR_mmap, sys_mmap)

/* mm/, CONFIG_MMU only */
#define __NR_mprotect 226
__SYSCALL(__NR_mprotect, sys_mprotect)

#define __NR_prlimit64 261
__SYSCALL(__NR_prlimit64, sys_prlimit64)

#define __NR_getrandom 278
__SYSCALL(__NR_getrandom, sys_getrandom)

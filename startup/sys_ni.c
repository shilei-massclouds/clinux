// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <sbi.h>
#include <errno.h>

/*  we can't #include <linux/syscalls.h> here,
    but tell gcc to not warn with -Wmissing-prototypes  */
long sys_ni_syscall(void);

/*
 * Non-implemented system calls get redirected here.
 */
long sys_ni_syscall(void)
{
    sbi_puts("non-implemented system!\n");
    halt();
    return -ENOSYS;
}

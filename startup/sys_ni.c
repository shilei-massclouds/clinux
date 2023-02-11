// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <sbi.h>
#include <errno.h>
#include <string.h>

/*  we can't #include <linux/syscalls.h> here,
    but tell gcc to not warn with -Wmissing-prototypes  */
long sys_ni_syscall(void);

/*
 * Non-implemented system calls get redirected here.
 */
long sys_ni_syscall(void)
{
    char buf[UL_STR_SIZE];
    register uintptr_t a7 asm ("a7");

    ul_to_str(a7, buf, sizeof(buf));
    sbi_puts("\nSyscall[0x");
    sbi_puts(buf);
    sbi_puts("] NOT-Implemented yet!\n");
    halt();
    return -ENOSYS;
}

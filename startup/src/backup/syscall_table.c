// SPDX-License-Identifier: GPL-2.0-only

#include <unistd.h>
#include <syscalls.h>

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (call),

void *sys_call_table[__NR_syscalls] = {
    [0 ... __NR_syscalls - 1] = sys_ni_syscall,
#include <asm_unistd.h>
};

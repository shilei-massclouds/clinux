// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <syscalls.h>

do_sys_open_t do_sys_open;
EXPORT_SYMBOL(do_sys_open);

SYSCALL_DEFINE4(openat, int, dfd, const char *, filename, int, flags,
                umode_t, mode)
{
    return do_sys_open(dfd, filename, flags, mode);
}

do_sys_readlinkat_t do_sys_readlinkat;
EXPORT_SYMBOL(do_sys_readlinkat);

SYSCALL_DEFINE4(readlinkat, int, dfd, const char *, pathname,
                char *, buf, int, bufsiz)
{
    return do_sys_readlinkat(dfd, pathname, buf, bufsiz);
}

do_sys_newuname_t do_sys_newuname;
EXPORT_SYMBOL(do_sys_newuname);

SYSCALL_DEFINE1(newuname, struct new_utsname *, name)
{
    return do_sys_newuname(name);
}

do_sys_brk_t do_sys_brk;
EXPORT_SYMBOL(do_sys_brk);

SYSCALL_DEFINE1(brk, unsigned long, brk)
{
    return do_sys_brk(brk);
}

do_sys_mprotect_t do_sys_mprotect;
EXPORT_SYMBOL(do_sys_mprotect);

SYSCALL_DEFINE3(mprotect, unsigned long, start, size_t, len,
                unsigned long, prot)
{
    return do_sys_mprotect(start, len, prot);
}

do_sys_mount_t do_sys_mount;
EXPORT_SYMBOL(do_sys_mount);

SYSCALL_DEFINE5(mount, char *, dev_name, char *, dir_name,
                char *, type, unsigned long, flags, void *, data)
{
    return do_sys_mount(dev_name, dir_name, type, flags, data);
}

ksys_write_t ksys_write;
EXPORT_SYMBOL(ksys_write);

SYSCALL_DEFINE3(write, unsigned int, fd, const char *, buf, size_t, count)
{
    sbi_puts("ksys_write ...\n");
    if (ksys_write == NULL) {
        sbi_puts("NOT register ksys_write yet!\n");
        sbi_srst_power_off();
    }
    return ksys_write(fd, buf, count);
}

do_faccessat_t do_faccessat;
EXPORT_SYMBOL(do_faccessat);

SYSCALL_DEFINE3(faccessat, int, dfd, const char *, filename, int, mode)
{
    return do_faccessat(dfd, filename, mode, 0);
}

do_group_exit_t do_group_exit;
EXPORT_SYMBOL(do_group_exit);

/*
 * this kills every thread in the thread group. Note that any externally
 * wait4()-ing process will get the correct exit code - even if this
 * thread is not the thread group leader.
 */
SYSCALL_DEFINE1(exit_group, int, error_code)
{
    if (do_group_exit == NULL) {
        sbi_puts("NOT register do_exit_group yet!\n");
        sbi_srst_power_off();
    }
    do_group_exit((error_code & 0xff) << 8);
    /* NOTREACHED */
    return 0;
}

do_set_tid_address_t do_set_tid_address;
EXPORT_SYMBOL(do_set_tid_address);

/* fork/fork.c */
SYSCALL_DEFINE1(set_tid_address, int *, tidptr)
{
    if (do_set_tid_address == NULL) {
        sbi_puts("NOT register do_set_tid_address yet!\n");
        sbi_srst_power_off();
    }
    return do_set_tid_address(tidptr);
}

/**
 * sys_set_robust_list() - Set the robust-futex list head of a task
 * @head:   pointer to the list-head
 * @len:    length of the list-head, as userspace expects
 */
SYSCALL_DEFINE2(set_robust_list,
                struct robust_list_head *, head,
                size_t, len)
{
    return -ENOSYS;
#if 0
    if (!futex_cmpxchg_enabled)
        return -ENOSYS;
    /*
     * The kernel knows only one size for now:
     */
    if (unlikely(len != sizeof(*head)))
        return -EINVAL;

    current->robust_list = head;

    return 0;
#endif
}

SYSCALL_DEFINE4(prlimit64, pid_t, pid, unsigned int, resource,
                const struct rlimit64 *, new_rlim,
                struct rlimit64 *, old_rlim)
{
    return -ENOSYS;
}

SYSCALL_DEFINE3(getrandom, char *, buf, size_t, count,
                unsigned int, flags)
{
    return -ENOSYS;
}

SYSCALL_DEFINE2(clock_gettime, const clockid_t, which_clock,
                struct __kernel_timespec *, tp)
{
    return 0;
}

SYSCALL_DEFINE6(mmap, unsigned long, addr, unsigned long, len,
                unsigned long, prot, unsigned long, flags,
                unsigned long, fd, off_t, offset)
{
    if (riscv_sys_mmap == NULL) {
        sbi_puts("NOT register riscv_sys_mmap yet!\n");
        sbi_srst_power_off();
    }
    return riscv_sys_mmap(addr, len, prot, flags, fd, offset, 0);
}

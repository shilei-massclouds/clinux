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
    if (ksys_write == NULL) {
        sbi_puts("NOT register ksys_write yet!\n");
        sbi_srst_power_off();
    }
    return ksys_write(fd, buf, count);
}

do_writev_t do_writev;
EXPORT_SYMBOL(do_writev);

SYSCALL_DEFINE3(writev, unsigned long, fd,
                const struct iovec *, vec,
                unsigned long, vlen)
{
    if (do_writev == NULL) {
        sbi_puts("NOT register do_writev yet!\n");
        sbi_srst_power_off();
    }
    return do_writev(fd, vec, vlen, 0);
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

SYSCALL_DEFINE2(munmap, unsigned long, addr, size_t, len)
{
    if (do_vm_munmap == NULL) {
        sbi_puts("NOT register do_vm_munmap yet!\n");
        sbi_srst_power_off();
    }

    addr = untagged_addr(addr);
    return do_vm_munmap(addr, len, true);
}

ksys_do_vfs_ioctl_t ksys_do_vfs_ioctl;
EXPORT_SYMBOL(ksys_do_vfs_ioctl);

SYSCALL_DEFINE3(ioctl, unsigned int, fd, unsigned int, cmd, unsigned long, arg)
{
    if (ksys_do_vfs_ioctl == NULL) {
        sbi_puts("NOT register ksys_do_vfs_ioctl yet!\n");
        sbi_srst_power_off();
    }

    return ksys_do_vfs_ioctl(fd, cmd, arg);
}

do_io_setup_t do_io_setup;
EXPORT_SYMBOL(do_io_setup);

/* sys_io_setup:
 *  Create an aio_context capable of receiving at least nr_events.
 *  ctxp must not point to an aio_context that already exists, and
 *  must be initialized to 0 prior to the call.  On successful
 *  creation of the aio_context, *ctxp is filled in with the resulting
 *  handle.  May fail with -EINVAL if *ctxp is not initialized,
 *  if the specified nr_events exceeds internal limits.  May fail
 *  with -EAGAIN if the specified nr_events exceeds the user's limit
 *  of available events.  May fail with -ENOMEM if insufficient kernel
 *  resources are available.  May fail with -EFAULT if an invalid
 *  pointer is passed for ctxp.  Will fail with -ENOSYS if not
 *  implemented.
 */
SYSCALL_DEFINE2(io_setup, unsigned, nr_events, aio_context_t *, ctxp)
{
    if (do_io_setup == NULL) {
        sbi_puts("NOT register do_io_setup yet!\n");
        sbi_srst_power_off();
    }

    return do_io_setup(nr_events, ctxp);
}

do_getpid_t do_getpid;
EXPORT_SYMBOL(do_getpid);

/**
 * sys_getpid - return the thread group id of the current process
 *
 * Note, despite the name, this returns the tgid not the pid.  The tgid and
 * the pid are identical unless CLONE_THREAD was specified on clone() in
 * which case the tgid is the same in all threads of the same group.
 *
 * This is SMP safe as current->tgid does not change.
 */
SYSCALL_DEFINE0(getpid)
{
    if (do_getpid == NULL) {
        sbi_puts("NOT register do_getpid yet!\n");
        sbi_srst_power_off();
    }

    return do_getpid();
}

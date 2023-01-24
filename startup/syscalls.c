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

SYSCALL_DEFINE3(write, unsigned int, fd,
                const char *, buf, size_t, count)
{
    return ksys_write(fd, buf, count);
}

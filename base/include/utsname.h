/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_UTSNAME_H
#define _UAPI_LINUX_UTSNAME_H

#include <current.h>
#include <nsproxy.h>

#define __NEW_UTS_LEN 64

struct new_utsname {
    char sysname[__NEW_UTS_LEN + 1];
    char nodename[__NEW_UTS_LEN + 1];
    char release[__NEW_UTS_LEN + 1];
    char version[__NEW_UTS_LEN + 1];
    char machine[__NEW_UTS_LEN + 1];
    char domainname[__NEW_UTS_LEN + 1];
};

struct uts_namespace {
    struct new_utsname name;
};

static inline struct new_utsname *utsname(void)
{
    return &current->nsproxy->uts_ns->name;
}

#endif /* _UAPI_LINUX_UTSNAME_H */

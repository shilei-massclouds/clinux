/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_NSPROXY_H
#define _LINUX_NSPROXY_H

struct nsproxy {
    struct uts_namespace *uts_ns;
};

#endif /* _LINUX_NSPROXY_H */

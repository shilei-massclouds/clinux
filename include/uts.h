/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_UTS_H
#define _LINUX_UTS_H

#define UTS_SYSNAME "Linux"
#define UTS_NODENAME CONFIG_DEFAULT_HOSTNAME /* set by sethostname() */
#define UTS_DOMAINNAME "(none)" /* set by setdomainname() */
#define UTS_RELEASE "5.9.0-rc4+"
#define UTS_VERSION "#1337 SMP Fri Mar 4 09:36:42 CST 2022"
#define UTS_MACHINE "riscv64"

#endif /* _LINUX_UTS_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_KBUILD_H
#define __LINUX_KBUILD_H

#include <types.h>

#define DEFINE(sym, val) \
    asm volatile("\n.ascii \"->" #sym " %0 " #val "\"" : : "i" (val))

#define OFFSET(sym, str, mem) \
    DEFINE(sym, offsetof(struct str, mem))

#endif /* __LINUX_KBUILD_H */

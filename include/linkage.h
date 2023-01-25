/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#include <page.h>
#include <compiler_attributes.h>

#define __initdata __section(.init.data)

#define __page_aligned_bss \
    __section(.bss..page_aligned) __aligned(PAGE_SIZE)

#endif /* _LINUX_LINKAGE_H */

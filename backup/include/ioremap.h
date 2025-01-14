// SPDX-License-Identifier: GPL-2.0-only
#ifndef _IOREMAP_H_
#define _IOREMAP_H_

#include <page.h>

void *
ioremap_prot(phys_addr_t addr, size_t size, unsigned long prot);

static inline void *
ioremap(phys_addr_t addr, size_t size)
{
    /* _PAGE_IOREMAP needs to be supplied by the architecture */
    return ioremap_prot(addr, size, _PAGE_IOREMAP);
}

#endif /* _IOREMAP_H_ */

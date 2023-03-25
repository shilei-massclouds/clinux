/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RUST_ALLOC_H
#define _LINUX_RUST_ALLOC_H

#include <types.h>

typedef char *
(*rg_alloc_t)(uintptr_t size, uintptr_t flags);
extern rg_alloc_t rg_alloc;

#endif /* _LINUX_RUST_ALLOC_H */

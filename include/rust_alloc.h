/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RUST_ALLOC_H
#define _LINUX_RUST_ALLOC_H

#include <types.h>

//fn __rust_alloc_zeroed(size: usize, align: usize) -> *mut u8;

typedef char *
(*rg_alloc_t)(uintptr_t size, uintptr_t align);
extern rg_alloc_t rg_alloc;

typedef void
(*rg_dealloc_t)(char *ptr, uintptr_t size, uintptr_t align);
extern rg_dealloc_t rg_dealloc;

typedef char *
(*rg_realloc_t)(char *ptr, uintptr_t old_size,
                uintptr_t align, uintptr_t new_size);
extern rg_realloc_t rg_realloc;

#endif /* _LINUX_RUST_ALLOC_H */

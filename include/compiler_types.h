/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_TYPES_H
#define __LINUX_COMPILER_TYPES_H

/* Compiler specific macros. */
#ifdef __clang__
#error "__clang__"
#elif defined(__INTEL_COMPILER)
#error "__INTEL_COMPILER"
#elif defined(__GNUC__)
/* The above compilers also define __GNUC__, so order is important here. */
#include <compiler-gcc.h>
#else
#error "Unknown compiler"
#endif

#endif /* __LINUX_COMPILER_TYPES_H */

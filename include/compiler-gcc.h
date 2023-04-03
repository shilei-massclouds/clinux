/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_TYPES_H
#error "Please don't include <linux/compiler-gcc.h> directly, include <linux/compiler.h> instead."
#endif

/*
 * Common definitions for all gcc versions go here.
 */
#define GCC_VERSION (__GNUC__ * 10000       \
             + __GNUC_MINOR__ * 100 \
             + __GNUC_PATCHLEVEL__)

/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58145 */
#if GCC_VERSION < 40900
# error Sorry, your compiler is too old - please upgrade it.
#endif

/* Optimization barrier */

/* The "volatile" is due to gcc bugs */
#define barrier() __asm__ __volatile__("": : :"memory")

/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SBI_H
#define _SBI_H

#include <types.h>
#include <compiler_attributes.h>

void sbi_putchar(int ch);

void sbi_puts(const char *s);
void sbi_put_u64(unsigned long n);

static __always_inline void
early_puts(unsigned long val)
{
    __asm__ __volatile__("csrw 0x0, %0\n" ::"r"(val):);
}

#endif /* _SBI_H */

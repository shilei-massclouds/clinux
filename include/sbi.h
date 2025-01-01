/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _SBI_H
#define _SBI_H

#include <types.h>
#include <compiler_attributes.h>

void sbi_putchar(int ch);

void sbi_puts(const char *s);
void sbi_put_u64(unsigned long n);

void sbi_srst_power_off(void);

#define SBI_ASSERT(cond) do {   \
    if (!(cond)) {              \
        sbi_puts("SBI ASSERT!\n"); \
        sbi_srst_power_off();   \
    }                           \
} while (0)

#define SBI_ASSERT_MSG(cond, msg) do {   \
    if (!(cond)) {              \
        sbi_puts("SBI ASSERT: " msg "\n"); \
        sbi_srst_power_off();   \
    }                           \
} while (0)

#endif /* _SBI_H */

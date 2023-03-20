/* SPDX-License-Identifier: GPL-2.0-only */

#define SBI_EXT_0_1_CONSOLE_PUTCHAR 0x1

typedef unsigned long   u64;

static void sbi_putchar(int ch)
{
    register u64 a0 asm ("a0") = (u64)ch;
    register u64 a7 asm ("a7") = (u64)SBI_EXT_0_1_CONSOLE_PUTCHAR;
    asm volatile ("ecall"
                  : "+r" (a0)
                  : "r" (a7)
                  : "memory");
}

static void sbi_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            sbi_putchar('\r');
        sbi_putchar(*s);
    }
}

void load_modules(void)
{
    sbi_puts("load_modules\n");
}

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <asm/sbi.h>
#include "booter.h"

#define UL_STR_SIZE 19  /* prefix with '0x' and end with '\0' */

void
start_kernel(void)
{
    load_modules();
    booter_panic();
}

void sbi_puts(const char *s)
{
    for (; *s; s++) {
        if (*s == '\n')
            sbi_console_putchar('\r');
        sbi_console_putchar(*s);
    }
}
EXPORT_SYMBOL(sbi_puts);

void sbi_put_u64(unsigned long n)
{
    char buf[UL_STR_SIZE];
    hex_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_u64);

void sbi_put_dec(unsigned long n)
{
    char buf[UL_STR_SIZE];
    dec_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_dec);

/*
void booter_panic(void)
{
    sbi_puts("\n########################\n");
    sbi_puts("PANIC: ");
    sbi_puts(__FUNCTION__);
    sbi_puts(" (");
    sbi_puts(__FILE__);
    sbi_puts(":");
    sbi_put_u64(__LINE__);
    sbi_puts(")");
    sbi_puts("\n########################\n");
    sbi_shutdown();
}
*/

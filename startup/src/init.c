// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <asm/sbi.h>
#include "booter.h"

#define UL_STR_SIZE 17  /* include '\0' */

void
start_kernel(void)
{
    booter_panic();
}

int
ul_to_str(unsigned long n, char *str, size_t len)
{
    int i;

    /* include '\0' */
    if (len != 17)
        return -1;

    for (i = 1; i <= 16; i++) {
        char c = (n >> ((16 - i)*4)) & 0xF;
        if (c >= 10) {
            c -= 10;
            c += 'A';
        } else {
            c += '0';
        }
        str[i-1] = c;
    }
    str[16] = '\0';

    return 0;
}
EXPORT_SYMBOL(ul_to_str);

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
    ul_to_str(n, buf, sizeof(buf));
    sbi_puts(buf);
}
EXPORT_SYMBOL(sbi_put_u64);

void booter_panic(void)
{
    sbi_puts("\n########################\n");
    //sbi_puts("PANIC: %s (%s:%u)\n", __FUNCTION__, __FILE__, __LINE__);
    //sbi_puts(args);
    sbi_puts("\n########################\n");
    sbi_shutdown();
}

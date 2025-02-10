// SPDX-License-Identifier: GPL-2.0-only

#include <linux/export.h>
#include <linux/cache.h>
#include <linux/jiffies.h>
#include <asm/sbi.h>
#include <asm/current.h>
#include <cl_hook.h>
#include "booter.h"

#define UL_STR_SIZE 19  /* prefix with '0x' and end with '\0' */

__visible u64 jiffies_64 __cacheline_aligned_in_smp = INITIAL_JIFFIES;
EXPORT_SYMBOL(jiffies_64);

extern int cl_init();

void
start_kernel(void)
{
    cl_init();
    booter_panic("Kernel has been terminated normally!");
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

DEFINE_HOOK(int, vprintk_func, const char *fmt, va_list args);

//
// Hook for printk
//
asmlinkage __visible __weak int printk(const char *fmt, ...)
{
    if (HAS_HOOK(vprintk_func)) {
        int ret;
        va_list args;
        va_start(args, fmt);
        INVOKE_HOOK_RET(ret, vprintk_func, printk_skip_level(fmt), args);
        va_end(args);
        return ret;
    }

    // Fallthrough
    sbi_puts("[RAW] ");
    sbi_puts(printk_skip_level(fmt));
    return 0;
}
EXPORT_SYMBOL(printk);

void __warn_printk(const char *fmt, ...)
{
    if (HAS_HOOK(vprintk_func)) {
        int ret;
        va_list args;
        pr_warn(CUT_HERE);
        va_start(args, fmt);
        INVOKE_HOOK_RET(ret, vprintk_func, printk_skip_level(fmt), args);
        va_end(args);
        return;
    }

    // Fallthrough
    sbi_puts("[RAW_WARN] ");
    sbi_puts(printk_skip_level(fmt));
}
EXPORT_SYMBOL(__warn_printk);

/**
 *  panic - halt the system
 *  @fmt: The text string to print
 *
 *  Display a message, then perform cleanups.
 *
 *  This function never returns.
 */
void __weak panic(const char *fmt, ...)
{
    sbi_puts("[RAW_PANIC] ");
    sbi_puts(fmt);
    sbi_shutdown();
    do {} while (1);
}
EXPORT_SYMBOL(panic);

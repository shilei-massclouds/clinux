// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include "../../booter/src/booter.h"

#if defined(CONFIG_PROVE_LOCKING) || defined(CONFIG_DEBUG_ATOMIC_SLEEP)
void __might_fault(const char *file, int line)
{
    sbi_puts("unimplemented __might_fault!");
    sbi_shutdown();
}
EXPORT_SYMBOL(__might_fault);
#endif

void __warn_printk(const char *fmt, ...)
{
    sbi_puts("unimplemented __warn_printk!");
    sbi_shutdown();
}
EXPORT_SYMBOL(__warn_printk);

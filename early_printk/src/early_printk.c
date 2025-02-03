// SPDX-License-Identifier: GPL-2.0-only

#include <stdarg.h>
#include <linux/console.h>
#include "../../booter/src/booter.h"

struct console *early_console;

int early_vprintk(const char *fmt, va_list args)
{
    int n;
    char buf[512];

    if (!early_console) {
        booter_panic("No early_console!");
    }

    n = vscnprintf(buf, sizeof(buf), fmt, args);
    early_console->write(early_console, buf, n);
}

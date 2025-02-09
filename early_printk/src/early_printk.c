// SPDX-License-Identifier: GPL-2.0-only

#include <stdarg.h>
#include <linux/console.h>
#include "../../booter/src/booter.h"

struct console *early_console;

int early_vprintk(const char *fmt, va_list args)
{
    int n;
    char buf[512];
    char *msg;

    if (!early_console) {
        booter_panic("No early_console!");
    }

    n = vscnprintf(buf, sizeof(buf), fmt, args);
	if (printk_get_level(buf)) {
		msg = buf + 2;
        n -= 2;
    } else {
        msg = buf;
    }
    early_console->write(early_console, msg, n);
}

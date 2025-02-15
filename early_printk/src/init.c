// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/serial_core.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

#define EARLYCON_NAME "sbi"

extern int early_sbi_setup(struct earlycon_device *device, const char *opt);
extern int early_vprintk(const char *fmt, va_list args);

static struct console early_con = {
    .name =     "uart",     /* fixed up at earlycon registration */
    .flags =    CON_PRINTBUFFER | CON_BOOT,
    .index =    0,
};

static struct earlycon_device early_console_dev = {
    .con = &early_con,
};

static void __init earlycon_init(struct earlycon_device *device,
                 const char *name)
{
    struct console *earlycon = device->con;
    struct uart_port *port = &device->port;
    const char *s;
    size_t len;

    /* scan backwards from end of string for first non-numeral */
    for (s = name + strlen(name);
         s > name && s[-1] >= '0' && s[-1] <= '9';
         s--)
        ;
    if (*s)
        earlycon->index = simple_strtoul(s, NULL, 10);
    len = s - name;
    strlcpy(earlycon->name, name, min(len + 1, sizeof(earlycon->name)));
    earlycon->data = &early_console_dev;

    /*
    if (port->iotype == UPIO_MEM || port->iotype == UPIO_MEM16 ||
        port->iotype == UPIO_MEM32 || port->iotype == UPIO_MEM32BE)
        pr_info("%s%d at MMIO%s %pa (options '%s')\n",
            earlycon->name, earlycon->index,
            (port->iotype == UPIO_MEM) ? "" :
            (port->iotype == UPIO_MEM16) ? "16" :
            (port->iotype == UPIO_MEM32) ? "32" : "32be",
            &port->mapbase, device->options);
    else
        pr_info("%s%d at I/O port 0x%lx (options '%s')\n",
            earlycon->name, earlycon->index,
            port->iobase, device->options);
    */
}

DEFINE_ENABLE_FUNC(early_printk);

int
cl_early_printk_init(void)
{
    int err;
    sbi_puts("module[early_printk]: init begin ...\n");

    earlycon_init(&early_console_dev, EARLYCON_NAME);
    err = early_sbi_setup(&early_console_dev, "");
    if (err < 0) {
        sbi_puts("early_sbi_setup err!");
        sbi_shutdown();
        //return err;
    }
    if (!early_console_dev.con->write) {
        sbi_puts("early_console ENODEV!");
        sbi_shutdown();
        //return -ENODEV;
    }

    early_console = early_console_dev.con;

    sbi_puts("module[early_printk]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_printk_init);

asmlinkage __visible __weak int printk(const char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = early_vprintk(printk_skip_level(fmt), args);
    va_end(args);
    return ret;
}
EXPORT_SYMBOL(printk);

asmlinkage __visible int cl_printk(const char *fmt, ...)
{
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = early_vprintk(printk_skip_level(fmt), args);
    va_end(args);
    return ret;
}
EXPORT_SYMBOL(cl_printk);

void __weak __warn_printk(const char *fmt, ...)
{
    int ret;
    va_list args;
    pr_warn(CUT_HERE);
    va_start(args, fmt);
    early_vprintk(printk_skip_level(fmt), args);
    va_end(args);
}
EXPORT_SYMBOL(__warn_printk);

// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/console.h>
#include <linux/serial_core.h>
#include <linux/linkage.h>
#include <linux/netdevice.h>
#include "../../booter/src/booter.h"

int console_printk[4] = {
	CONSOLE_LOGLEVEL_DEFAULT,	/* console_loglevel */
	MESSAGE_LOGLEVEL_DEFAULT,	/* default_message_loglevel */
	CONSOLE_LOGLEVEL_MIN,		/* minimum_console_loglevel */
	CONSOLE_LOGLEVEL_DEFAULT,	/* default_console_loglevel */
};
EXPORT_SYMBOL_GPL(console_printk);

/*
 * System may need to suppress printk message under certain
 * circumstances, like after kernel panic happens.
 */
int __read_mostly suppress_printk;
EXPORT_SYMBOL(suppress_printk);

__weak asmlinkage __visible int printk(const char *fmt, ...)
{
}
EXPORT_SYMBOL(printk);

__weak noinstr int debug_locks_off(void)
{
    return 0;
}
EXPORT_SYMBOL_GPL(debug_locks_off);

__weak asmlinkage int vprintk_emit(int facility, int level,
			    const char *dict, size_t dictlen,
			    const char *fmt, va_list args)
{
}
EXPORT_SYMBOL(vprintk_emit);

__weak struct tty_driver *console_device(int *index)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_device);

__weak asmlinkage int vprintk(const char *fmt, va_list args)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(vprintk);

__weak void console_unblank(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_unblank);

__weak void
netdev_printk(const char *level,
              const struct net_device *dev,
              const char *format, ...)
{
}
EXPORT_SYMBOL(netdev_printk);

#define define_netdev_printk_level_weak_func(func, level)			 \
__weak void func(const struct net_device *dev, const char *fmt, ...) \
{								\
}								\
EXPORT_SYMBOL(func);

define_netdev_printk_level_weak_func(netdev_emerg, KERN_EMERG);
define_netdev_printk_level_weak_func(netdev_alert, KERN_ALERT);
define_netdev_printk_level_weak_func(netdev_crit, KERN_CRIT);
define_netdev_printk_level_weak_func(netdev_err, KERN_ERR);
define_netdev_printk_level_weak_func(netdev_warn, KERN_WARNING);
define_netdev_printk_level_weak_func(netdev_notice, KERN_NOTICE);
define_netdev_printk_level_weak_func(netdev_info, KERN_INFO);

__weak void
uart_console_write(struct uart_port *port, const char *s,
                   unsigned int count,
                   void (*putchar)(struct uart_port *, int))
{
}
EXPORT_SYMBOL_GPL(uart_console_write);

__weak const char *dev_driver_string(const struct device *dev)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(dev_driver_string);

int
cl_printk_itf_init(void)
{
    sbi_puts("module[printk_itf]: init begin ...\n");
    sbi_puts("module[printk_itf]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_printk_itf_init);


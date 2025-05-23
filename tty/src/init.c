// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_tty_init(void)
{
    sbi_puts("module[tty]: init begin ...\n");
    sbi_puts("module[tty]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tty_init);

/*
ssize_t vfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    booter_panic("No impl.\n");
}
*/
int ptm_open_peer(struct file *master, struct tty_struct *tty, int flags)
{
    booter_panic("No impl.\n");
}

struct tty_driver *console_driver;
EXPORT_SYMBOL(console_driver);

// From kernel/printk/printk.c
struct console *console_drivers;
EXPORT_SYMBOL_GPL(console_drivers);

void __weak console_lock(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_lock);

void __weak console_unlock(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_unlock);

/*
 * fg_console is the current virtual console,
 * last_console is the last used one,
 * want_console is the console we want to switch to,
 * saved_* variants are for save/restore around kernel debugger enter/leave
 */
int fg_console;
EXPORT_SYMBOL(fg_console);

__weak int __init vty_init(const struct file_operations *console_fops)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(vty_init);

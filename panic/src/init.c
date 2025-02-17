// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include <linux/reboot.h>
#include <linux/console.h>
#include <linux/kmsg_dump.h>
#include <linux/mm.h>
#include "../../booter/src/booter.h"

int
cl_panic_init(void)
{
    sbi_puts("module[panic]: init begin ...\n");
    sbi_puts("module[panic]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_panic_init);

//int __read_mostly suppress_printk;

#define DEFAULT_REBOOT_MODE
enum reboot_mode reboot_mode DEFAULT_REBOOT_MODE;
enum reboot_mode panic_reboot_mode = REBOOT_UNDEFINED;

void sysrq_timer_list_show(void)
{
    booter_panic("No impl!\n");
}

/*
void bust_spinlocks(int yes)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(bust_spinlocks);
*/
__weak void wake_up_klogd(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(wake_up_klogd);

void show_state_filter(unsigned long state_filter)
{
    booter_panic("No impl!\n");
}

__weak void printk_safe_flush_on_panic(void)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(printk_safe_flush_on_panic);

void emergency_restart(void)
{
    booter_panic("No impl!\n");
}

void console_unblank(void)
{
    sbi_puts("========================== console_unblank\n");
    //booter_panic("No impl!\n");
}

void unblank_screen(void)
{
    sbi_puts("========================== unblank_screen\n");
    //booter_panic("No impl!\n");
}

void console_flush_on_panic(enum con_flush_mode mode)
{
    sbi_puts("========================== console_flush_on_panic\n");
    //booter_panic("No impl!\n");
}

__weak void kmsg_dump(enum kmsg_dump_reason reason)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(kmsg_dump);

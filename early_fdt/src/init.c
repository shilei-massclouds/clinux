// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <asm/setup.h>
#include "../../booter/src/booter.h"

unsigned long initrd_start, initrd_end;
int initrd_below_start_ok;
phys_addr_t phys_initrd_start __initdata;
unsigned long phys_initrd_size __initdata;

/* Untouched command line saved by arch-specific code. */
char __initdata boot_command_line[COMMAND_LINE_SIZE];

int
cl_early_fdt_init(void)
{
    sbi_puts("module[early_fdt]: init begin ...\n");
    sbi_puts("module[early_fdt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_fdt_init);

void add_bootloader_randomness(const void *buf, unsigned int size)
{
    booter_panic("No impl 'add_bootloader_randomness'.");
}

int __init_memblock memblock_add(phys_addr_t base, phys_addr_t size)
{
    booter_panic("No impl 'memblock_add'.");
}

int __init_memblock memblock_mark_hotplug(phys_addr_t base, phys_addr_t size)
{
    booter_panic("No impl 'memblock_mark_hotplug'.");
}

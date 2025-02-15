// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/serial_core.h>
#include <asm/setup.h>
#include "../../booter/src/booter.h"

extern void *dtb_early_va;

unsigned long initrd_start;
EXPORT_SYMBOL(initrd_start);
unsigned long initrd_end;
EXPORT_SYMBOL(initrd_end);
int initrd_below_start_ok;
EXPORT_SYMBOL(initrd_below_start_ok);

phys_addr_t phys_initrd_start __initdata;
EXPORT_SYMBOL(phys_initrd_start);
unsigned long phys_initrd_size __initdata;
EXPORT_SYMBOL(phys_initrd_size);

/* Untouched command line saved by arch-specific code. */
char __initdata boot_command_line[COMMAND_LINE_SIZE];
EXPORT_SYMBOL(boot_command_line);

int
cl_early_fdt_init(void)
{
    sbi_puts("module[early_fdt]: init begin ...\n");
    sbi_puts("module[early_fdt]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_early_fdt_init);

void parse_dtb(void)
{
    if (early_init_dt_scan(dtb_early_va))
        return;

    booter_panic("No DTB passed to the kernel\n");
}
EXPORT_SYMBOL(parse_dtb);

void __weak add_bootloader_randomness(const void *buf, unsigned int size)
{
    booter_panic("Need random driver take effect!\n");
}
EXPORT_SYMBOL_GPL(add_bootloader_randomness);

#ifdef CONFIG_OF_EARLY_FLATTREE

int __init __weak of_setup_earlycon(const struct earlycon_id *match,
                 unsigned long node,
                 const char *options)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(of_setup_earlycon);

#endif

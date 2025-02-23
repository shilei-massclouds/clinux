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

__weak int of_property_read_variable_u32_array(const struct device_node *np,
			       const char *propname, u32 *out_values,
			       size_t sz_min, size_t sz_max)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL_GPL(of_property_read_variable_u32_array);

__weak bool of_device_is_available(const struct device_node *device)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(of_device_is_available);

__weak struct device_node *of_find_matching_node_and_match(struct device_node *from,
					const struct of_device_id *matches,
					const struct of_device_id **match)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(of_find_matching_node_and_match);

__weak struct device_node *of_find_node_opts_by_path(const char *path, const char **opts)
{
    booter_panic("No impl!\n");
}
EXPORT_SYMBOL(of_find_node_opts_by_path);

__weak void __init of_core_init(void)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(of_core_init);

__weak struct device_node *of_get_next_cpu_node(struct device_node *prev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(of_get_next_cpu_node);

__weak struct device_node *of_get_cpu_node(int cpu, unsigned int *thread)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(of_get_cpu_node);

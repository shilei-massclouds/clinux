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
/* Untouched extra command line */
static char *extra_command_line;
/* Extra init arguments */
char *extra_init_args;
EXPORT_SYMBOL(extra_init_args);

/* Command line for parameter parsing */
char *static_command_line;
EXPORT_SYMBOL(static_command_line);

#ifdef CONFIG_BOOT_CONFIG
/* Is bootconfig on command line? */
static bool bootconfig_found;
static bool initargs_found;
#else
# define bootconfig_found false
# define initargs_found false
#endif

/* Untouched command line saved by arch-specific code. */
char __initdata boot_command_line[COMMAND_LINE_SIZE];
EXPORT_SYMBOL(boot_command_line);

extern void __init setup_command_line(void);

int
cl_early_fdt_init(void)
{
    sbi_puts("module[early_fdt]: init begin ...\n");
    if (strlen(boot_command_line) == 0) {
        booter_panic("NOT setup boot_command_line yet.");
    }
    setup_command_line();
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

__weak int of_property_read_string(const struct device_node *np, const char *propname,
				const char **out_string)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(of_property_read_string);

__weak int of_device_is_compatible(const struct device_node *device,
		const char *compat)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(of_device_is_compatible);

__weak void of_device_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(of_device_uevent);

__weak bool of_dma_is_coherent(struct device_node *np)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(of_dma_is_coherent);

__weak int of_address_to_resource(struct device_node *dev, int index,
			   struct resource *r)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(of_address_to_resource);

/*
 * We need to store the untouched command line for future reference.
 * We also need to store the touched command line since the parameter
 * parsing is performed in place, and we should allow a component to
 * store reference of name/value for future reference.
 */
void __init setup_command_line(void)
{
	size_t len, xlen = 0, ilen = 0;

	if (extra_command_line)
		xlen = strlen(extra_command_line);
	if (extra_init_args)
		ilen = strlen(extra_init_args) + 4; /* for " -- " */

	len = xlen + strlen(boot_command_line) + 1;

	saved_command_line = memblock_alloc(len + ilen, SMP_CACHE_BYTES);
	if (!saved_command_line)
		panic("%s: Failed to allocate %zu bytes\n", __func__, len + ilen);

	static_command_line = memblock_alloc(len, SMP_CACHE_BYTES);
	if (!static_command_line)
		panic("%s: Failed to allocate %zu bytes\n", __func__, len);

	if (xlen) {
		/*
		 * We have to put extra_command_line before boot command
		 * lines because there could be dashes (separator of init
		 * command line) in the command lines.
		 */
		strcpy(saved_command_line, extra_command_line);
		strcpy(static_command_line, extra_command_line);
	}
	strcpy(saved_command_line + xlen, boot_command_line);
	strcpy(static_command_line + xlen, boot_command_line);

	if (ilen) {
		/*
		 * Append supplemental init boot args to saved_command_line
		 * so that user can check what command line options passed
		 * to init.
		 */
		len = strlen(saved_command_line);
		if (initargs_found) {
			saved_command_line[len++] = ' ';
		} else {
			strcpy(saved_command_line + len, " -- ");
			len += 4;
		}

		strcpy(saved_command_line + len, extra_init_args);
	}
}
EXPORT_SYMBOL(setup_command_line);

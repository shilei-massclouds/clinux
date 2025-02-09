// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <linux/smp.h>
#include <linux/cgroup.h>
#include <linux/cpu.h>
#include <linux/security.h>
#include <linux/of_fdt.h>
#include <linux/dma-direct.h>
#include <linux/initrd.h>
#include <asm/pgtable.h>
#include <asm/sbi.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

/* Untouched saved command line (eg. for /proc) */
char *saved_command_line;

/* Command line for parameter parsing */
static char *static_command_line;
/* Untouched extra command line */
static char *extra_command_line;
/* Extra init arguments */
static char *extra_init_args;

#ifdef CONFIG_BOOT_CONFIG
/* Is bootconfig on command line? */
static bool bootconfig_found;
static bool initargs_found;
#else
# define bootconfig_found false
# define initargs_found false
#endif

void __init setup_arch(char **cmdline_p)
{
    setup_kernel_in_mm();
    *cmdline_p = boot_command_line;

    parse_early_param();

    setup_bootmem();
    paging_init();
#if IS_ENABLED(CONFIG_BUILTIN_DTB)
    unflatten_and_copy_device_tree();
#else
    unflatten_device_tree();
#endif

#ifdef CONFIG_SWIOTLB
    swiotlb_init(1);
#endif

#ifdef CONFIG_KASAN
    kasan_init();
#endif

#if IS_ENABLED(CONFIG_RISCV_SBI)
    sbi_init();
#endif

#ifdef CONFIG_SMP
    setup_smp();
#endif

    riscv_fill_hwcap();
}

#ifdef CONFIG_BLK_DEV_INITRD
static void * __init get_boot_config_from_initrd(u32 *_size, u32 *_csum)
{
    u32 size, csum;
    char *data;
    u32 *hdr;

    if (!initrd_end)
        return NULL;

    panic("Cannot reach here!\n");
}
#else
static void * __init get_boot_config_from_initrd(u32 *_size, u32 *_csum)
{
    return NULL;
}
#endif

#ifdef CONFIG_BOOT_CONFIG
#error bad config 'CONFIG_BOOT_CONFIG'
#else
static void __init setup_boot_config(const char *cmdline)
{
    /* Remove bootconfig data from initrd */
    get_boot_config_from_initrd(NULL, NULL);
}
#endif

/*
 * We need to store the untouched command line for future reference.
 * We also need to store the touched command line since the parameter
 * parsing is performed in place, and we should allow a component to
 * store reference of name/value for future reference.
 */
static void __init setup_command_line(char *command_line)
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
	strcpy(static_command_line + xlen, command_line);

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

int
cl_top_linux_init(void)
{
    char *command_line;

    sbi_puts("module[top_linux]: init begin ...\n");
    REQUIRE_COMPONENT(early_printk);

    //
    // start_kernel (init/main.c)
    //

    REQUIRE_COMPONENT(task); // Setup 'tp => init_task' here from head.S.
    parse_dtb();             // Move 'parse_dtb' here from head.S.

    set_task_stack_end_magic(&init_task);
    smp_setup_processor_id();
    debug_objects_early_init();

    cgroup_init_early();

    local_irq_disable();
    early_boot_irqs_disabled = true;

    /*
     * Interrupts are still disabled. Do necessary setups, then
     * enable them.
     */
    boot_cpu_init();
    page_address_init();
    pr_notice("%s", linux_banner);
    early_security_init();
    setup_arch(&command_line);
    setup_boot_config(command_line);
    setup_command_line(command_line);

    /*
    setup_nr_cpu_ids();
    setup_per_cpu_areas();
    */
    //smp_prepare_boot_cpu(); /* arch-specific boot-cpu hooks */
    //boot_cpu_hotplug_init();


    printk("command_line: '%s'\n", command_line);
    sbi_puts("module[top_linux]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_linux_init);

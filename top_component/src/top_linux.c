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
#include <asm/pgtable.h>
#include <asm/sbi.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

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

    printk("command_line: '%s'\n", command_line);
    sbi_puts("module[top_linux]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_linux_init);

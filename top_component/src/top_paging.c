// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <asm/pgtable.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_paging_init(void)
{
    sbi_puts("module[top_paging]: init begin ...\n");

    //
    // start_kernel (init/main.c)
    //

    REQUIRE_COMPONENT(task); // Setup 'tp => init_task' here from head.S.
    parse_dtb();             // Move 'parse_dtb' here from head.S.

    set_task_stack_end_magic(&init_task);
    smp_setup_processor_id();
    debug_objects_early_init();

    // Note: disable cgroup.
    //cgroup_init_early();

    local_irq_disable();
    early_boot_irqs_disabled = true;

    setup_kernel_in_mm();
    parse_early_param();
    setup_bootmem();
    sbi_puts("module[top_paging]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_paging_init);

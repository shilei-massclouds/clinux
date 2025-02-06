// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <asm/pgtable.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

int
cl_top_linux_init(void)
{
    sbi_puts("module[top_linux]: init begin ...\n");
    REQUIRE_COMPONENT(early_printk);

    //
    // start_kernel (init/main.c)
    //

    REQUIRE_COMPONENT(task); // Setup 'tp => init_task' here from head.S.
    parse_dtb();             // Move 'parse_dtb' here from head.S.

    set_task_stack_end_magic(&init_task);

    setup_kernel_in_mm();
    parse_early_param();
    setup_bootmem();
    paging_init();
    sbi_puts("module[top_linux]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_linux_init);

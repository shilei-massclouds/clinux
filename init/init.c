// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <fs.h>
#include <bug.h>
#include <fdt.h>
#include <printk.h>
#include <platform.h>

extern uintptr_t kernel_size;
extern void arch_call_rest_init(void);

static void
start_kernel(void)
{
    printk("start_kernel: init ...\n");

    if (kernel_size >= PMD_SIZE)
        panic("kernel size (%lu) is over PME_SIZE!", kernel_size);

    /* Do the rest non-__init'ed, we're now alive */
    arch_call_rest_init();

    printk("start_kernel: init ok!\n");
}

static int
init_module(void)
{
    printk("module[init]: init begin ...\n");

    BUG_ON(!rootfs_initialized);
    start_kernel_fn = start_kernel;

    printk("module[init]: init end!\n");

    return 0;
}

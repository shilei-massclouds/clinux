// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <bug.h>
#include <slab.h>
#include <printk.h>

extern bool sys_ready;

#define UNI_PROCESS "payload://test"

static const char *argv_init[] = { NULL, };
const char *envp_init[] = { NULL, };

static int
run_uniprocess(const char *filename)
{
    const char *const *p;

    pr_info("\nRun '%s' as uni-process\n", filename);
    return kernel_execve(filename, argv_init, envp_init);
}

int
init_module(void)
{
    printk("module[top_unikernel]: init begin ...\n");
    BUG_ON(!slab_is_available());
    BUG_ON(!sys_ready);
    BUG_ON(!rootfs_initialized);

    /*
    {
        extern void *sys_call_table[];
        printk("%s: sys_call_table %p\n", __func__, sys_call_table[0]);

        typedef long (*sys_io_setup_t)(unsigned long, unsigned long);
        sys_io_setup_t func = (sys_io_setup_t) sys_call_table[0];
        func(0, 0);
    }
    */

    run_uniprocess(UNI_PROCESS);

    printk("module[top_unikernel]: init end!\n");
    return 0;
}

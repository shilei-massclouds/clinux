// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <slab.h>
#include <printk.h>

extern bool axruntime_ready(void);

int
init_module(void)
{
    printk("module[libax]: init begin ...\n");
    BUG_ON(!axruntime_ready());
    BUG_ON(!slab_is_available());
    printk("module[libax]: init end!\n");
    return 0;
}

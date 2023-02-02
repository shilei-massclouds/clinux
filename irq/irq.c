// SPDX-License-Identifier: GPL-2.0+

#include <printk.h>
#include <irqflags.h>

static int
init_module(void)
{
    printk("module[irq]: init begin ...\n");

    local_irq_enable();

    printk("module[irq]: init end!\n");
    return 0;
}

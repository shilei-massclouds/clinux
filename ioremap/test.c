// SPDX-License-Identifier: GPL-2.0-only
#include <printk.h>
#include <string.h>
#include <ioremap.h>

static int
test_ioremap(void)
{
    void *vaddr;

    vaddr = ioremap(0x10001000, 0x1000);
    if (!vaddr)
        return -1;

    printk(_GREEN("[%lx]\n"), vaddr);
    return 0;
}

static int
init_module(void)
{
    printk("module[test_ioremap]: init begin ...\n");

    if(test_ioremap()) {
        printk(_RED("ioremap failed!\n"));
        return -1;
    }

    printk(_GREEN("ioremap okay!\n"));

    printk("module[test_ioremap]: init end!\n");
    return 0;
}

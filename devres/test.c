// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <device.h>
#include <devres.h>
#include <ioport.h>
#include <printk.h>
#include <string.h>
#include <ioremap.h>

static int
test_devres(void)
{
    void *vaddr;
    struct resource *res;
    struct device *dev;

    res = kzalloc(sizeof(struct resource), GFP_KERNEL);
    if (!res)
        return -1;

    res->start = 0x10000100;
    res->end = 0x10000200;
    res->name = "test_res";

    dev = kzalloc(sizeof(struct device), GFP_KERNEL);
    if (!dev)
        return -1;

    vaddr = devm_ioremap_resource(dev, res);
    if (!vaddr)
        return -1;

    printk(_GREEN("[%lx]\n"), vaddr);
    return 0;
}

static int
init_module(void)
{
    printk("module[test_devres]: init begin ...\n");

    if(test_devres()) {
        printk(_RED("devres failed!\n"));
        return -1;
    }

    printk(_GREEN("devres okay!\n"));

    printk("module[test_devres]: init end!\n");
    return 0;
}

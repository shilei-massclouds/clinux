// SPDX-License-Identifier: GPL-2.0-only

#include <klist.h>
#include <printk.h>
#include <platform.h>

struct platform_device *example = NULL;

static int
test_traverse(void)
{
    struct klist_iter iter;
    struct klist_node *n;
    struct bus_type *bus = &platform_bus_type;

    if (!bus || !bus->p)
        return -1;

    klist_iter_init_node(&bus->p->klist_devices, &iter, NULL);
    while ((n = klist_next(&iter))) {
        int i;
        struct device_private *dev_prv = to_device_private_bus(n);
        struct device *dev = dev_prv->device;
        struct platform_device *pdev = to_platform_device(dev);
        struct device_node *of_node = dev->of_node;
        struct property *property = of_node->properties;
        printk("device: %s.\n", of_node->full_name);
        printk("  of porperties:\n");
        while (property) {
            printk("    [%s]\n", property->name);
            property = property->next;
        }
        printk("  resources (%u):\n", pdev->num_resources);
        for (i = 0; i < pdev->num_resources; i++) {
            printk("    resources (%lx, %lx):\n",
                   pdev->resource[i].start, pdev->resource[i].end);
        }

        if (!strcmp(of_node->full_name, "virtio_mmio@10001000")) {
            example = pdev;
        }
    }
    klist_iter_exit(&iter);

    return 0;
}

static int
test_ioremap(void)
{
    void *base;
    struct resource *r;

    if (example == NULL) {
        printk("%s: example not init yet!\n", __func__);
        return -1;
    }

    r = platform_get_resource(example, IORESOURCE_MEM, 0);
    if (r == NULL)
        return -1;

    printk("\nFor example: device virtio_mmio@10001000.\n");
    printk("    resource0 flags(%lx) (%lx, %lx):\n",
           r->flags, r->start, r->end);

    base = devm_platform_ioremap_resource(example, 0);
    printk("    ioremap vaddr(%lx)\n\n", base);
    return 0;
}

static int
init_module(void)
{
    printk("module[test_platform]: init begin ...\n");

    if(test_traverse()) {
        printk(_RED("test traverse failed!\n"));
        return -1;
    }

    if(test_ioremap()) {
        printk(_RED("test ioremap failed!\n"));
        return -1;
    }

    printk(_GREEN("test platform okay!\n"));

    printk("module[test_platform]: init end!\n");
    return 0;
}

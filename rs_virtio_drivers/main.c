// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <export.h>
#include <ioremap.h>

unsigned long VIRTIO_BASE_ADDR = 0;
EXPORT_SYMBOL(VIRTIO_BASE_ADDR);

int
init_module(void)
{
    phys_addr_t base;

    printk("module[virtio_drivers]: init begin ...\n");

    for (base = 0x10001000; base <= 0x10008000; base += 0x1000) {
        void *vaddr = ioremap(base, 0x1000);
        printk("ioremap: [%p]\n", vaddr);
        if (VIRTIO_BASE_ADDR == 0)
            VIRTIO_BASE_ADDR = (unsigned long) vaddr;
    }

    printk("module[virtio_drivers]: init end!\n");
    return 0;
}

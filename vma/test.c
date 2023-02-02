// SPDX-License-Identifier: GPL-2.0-only

#include <vma.h>
#include <printk.h>
#include <string.h>

static int
test_get_vma(void)
{
    struct vm_struct *area;

    area = get_vm_area(256, 0);
    if (!area)
        return -1;

    printk(_GREEN("[%lx] - %lx\n"), area->addr, area->size);
    return 0;
}

static int
init_module(void)
{
    printk("module[test_get_vma]: init begin ...\n");

    if(test_get_vma()) {
        printk(_RED("get vma failed!\n"));
        return -1;
    }

    printk(_GREEN("get vma okay!\n"));

    printk("module[test_get_vma]: init end!\n");
    return 0;
}

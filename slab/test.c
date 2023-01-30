// SPDX-License-Identifier: GPL-2.0-only
#include <slab.h>
#include <printk.h>
#include <string.h>

static int
kmalloc_specific_size(int size)
{
    void *p;

    p = kmalloc(size, GFP_KERNEL);
    if (p == NULL) {
        printk(_RED("kmalloc size:%d failed!\n"), size);
        return -1;
    }
    memset(p, 0, size);

    return 0;
}

static int
test_kmalloc(void)
{
    int i;

    for (i = 8; i < KMALLOC_MAX_SIZE; i += 8) {
        printk("%d\n", i);
        if (kmalloc_specific_size(i-1))
            return -1;

        if (kmalloc_specific_size(i))
            return -1;

        if (kmalloc_specific_size(i+1))
            return -1;
    }

    for (i = 0; i < 2; i++) {
        if (kmalloc_specific_size(0x200000-1))
            return -1;
    }

    return 0;
}

static int
test_kfree(void)
{
    void *p0;
    void *p1;
    void *p2;

    kfree(NULL);

    p0 = kmalloc(37, GFP_KERNEL);
    if (p0 == NULL)
        return -1;
    kfree(p0);

    p1 = kmalloc(38, GFP_KERNEL);
    p2 = kmalloc(38, GFP_KERNEL);
    if (p1 == NULL || p2 == NULL)
        return -1;
    kfree(p1);
    kfree(p2);

    return 0;
}

static int
init_module(void)
{
    printk("module[test_slab]: init begin ...\n");
    printk("test slab alloc ...\n");

    if (test_kmalloc())
        printk(_RED("test slab alloc failed!\n"));
    else
        printk(_GREEN("test slab alloc ok!\n"));

    if (test_kfree())
        printk(_RED("test slab free failed!\n"));
    else
        printk(_GREEN("test slab free ok!\n"));

    printk("module[test_slab]: init end!\n");

    return 0;
}

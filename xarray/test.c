// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <string.h>
#include <xarray.h>

static int
test_xas_load(struct xarray *xarray, pgoff_t offset)
{
    void *entry;
    XA_STATE(xas, xarray, offset);

    entry = xas_load(&xas);
    if (entry) {
        printk(_RED("%lx already exist!\n"), offset);
        return -1;
    }

    printk(_GREEN("%lx now is NULL!\n"), offset);
    return 0;
}

static int
test_xas_store(struct xarray *xarray, pgoff_t offset, void *entry)
{
    void *entry2;
    XA_STATE(xas, xarray, offset);
    XA_STATE(xas2, xarray, offset);

    xas_store(&xas, entry);
    if (xas_error(&xas)) {
        printk(_RED("store [%lx]:%p failed!\n"), offset, entry);
        return -1;
    }

    entry2 = xas_load(&xas2);
    if (!entry) {
        printk(_RED("no [%lx]!\n"), offset);
        return -1;
    }

    printk(_GREEN("store [%lx]:(%p, %p) okay!\n"), offset, entry, entry2);
    return 0;
}

static int
test1(void)
{
    void *entry;
    struct xarray xarray;
    unsigned long dummy[2];

    memset(&xarray, 0, sizeof(xarray));

    if (test_xas_load(&xarray, 0)) {
        printk(_RED("xa_load 0 error!"));
        return -1;
    }

    if (test_xas_load(&xarray, 0x1234)) {
        printk(_RED("xa_load 0x1234 error!"));
        return -1;
    }

    entry = (void *) &dummy[0];
    if (test_xas_store(&xarray, 0, entry)) {
        printk(_RED("xa_store [0]:%p error!"), entry);
        return -1;
    }

    entry = (void *) &dummy[1];
    if (test_xas_store(&xarray, 0x1234, entry)) {
        printk(_RED("xa_store [0x1234]:%p error!"), entry);
        return -1;
    }

    return 0;
}

static int
test2(void)
{
    void *entry;
    struct xarray xarray;
    unsigned long dummy[2];

    memset(&xarray, 0, sizeof(xarray));

    if (test_xas_load(&xarray, 0x1234)) {
        printk(_RED("xa_load 0x1234 error!"));
        return -1;
    }

    if (test_xas_load(&xarray, 0)) {
        printk(_RED("xa_load 0 error!"));
        return -1;
    }

    entry = (void *) &dummy[0];
    if (test_xas_store(&xarray, 0x1234, entry)) {
        printk(_RED("xa_store [0x1234]:%p error!"), entry);
        return -1;
    }

    entry = (void *) &dummy[1];
    if (test_xas_store(&xarray, 0, entry)) {
        printk(_RED("xa_store [0]:%p error!"), entry);
        return -1;
    }

    return 0;
}

static int
init_module(void)
{
    printk("module[test_xarray]: init begin ...\n");

    printk("test1 ...\n");
    test1();

    printk("test2 ...\n");
    test2();

    printk("module[test_xarray]: init end!\n");
    return 0;
}

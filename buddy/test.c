// SPDX-License-Identifier: GPL-2.0-only
#include <gfp.h>
#include <printk.h>

static int
test_alloc_pages(void)
{
    int i;

    for (i = 0; i < MAX_ORDER; i++) {
        struct page *page;
        page = alloc_pages(GFP_KERNEL, i);
        if(!page)
            return -1;

        printk("alloc order(%d) ok!\n", i);
    }

    return 0;
}

static int
init_module(void)
{
    printk("module[test_buddy]: init begin ...\n");

    if(test_alloc_pages())
        printk(_RED("alloc pages failed!\n"));
    else
        printk(_GREEN("alloc pages okay!\n"));

    printk("module[test_buddy]: init end!\n");
    return 0;
}

// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <slab.h>
#include <export.h>
#include <printk.h>
#include <backing-dev.h>

struct backing_dev_info noop_backing_dev_info = {
    .capabilities = BDI_CAP_NO_ACCT_AND_WRITEBACK,
};
EXPORT_SYMBOL(noop_backing_dev_info);

static int bdi_init(struct backing_dev_info *bdi)
{
    /*
    bdi->dev = NULL;

    bdi->min_ratio = 0;
    bdi->max_ratio = 100;
    bdi->max_prop_frac = FPROP_FRAC_BASE;
    INIT_LIST_HEAD(&bdi->bdi_list);
    INIT_LIST_HEAD(&bdi->wb_list);
    init_waitqueue_head(&bdi->wb_waitq);
    */
    return 0;
}

struct backing_dev_info *bdi_alloc(void)
{
    struct backing_dev_info *bdi;

    bdi = kzalloc_node(sizeof(*bdi), GFP_KERNEL);
    if (!bdi)
        panic("out of memory!");

    if (bdi_init(bdi)) {
        kfree(bdi);
        return NULL;
    }
    return bdi;
}
EXPORT_SYMBOL(bdi_alloc);

int
init_module(void)
{
    printk("module[backing-dev]: init begin ...\n");
    printk("module[backing-dev]: init end!\n");
    return 0;
}

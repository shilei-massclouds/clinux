// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/flex_proportions.h>
#include "../../booter/src/booter.h"

int
cl_backing_dev_init(void)
{
    sbi_puts("module[backing_dev]: init begin ...\n");
    sbi_puts("module[backing_dev]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_backing_dev_init);

bool flush_delayed_work(struct delayed_work *dwork)
{
    booter_panic("No impl!\n");
}
void debugfs_remove(struct dentry *dentry)
{
    booter_panic("No impl!\n");
}
bool mod_delayed_work_on(int cpu, struct workqueue_struct *wq,
             struct delayed_work *dwork, unsigned long delay)
{
    booter_panic("No impl!\n");
}
void fprop_local_destroy_percpu(struct fprop_local_percpu *pl)
{
    booter_panic("No impl!\n");
}

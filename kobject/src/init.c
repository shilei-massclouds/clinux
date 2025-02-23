// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include "../../booter/src/booter.h"

int
cl_kobject_init(void)
{
    sbi_puts("module[kobject]: init begin ...\n");
    sbi_puts("module[kobject]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_kobject_init);

__weak void sysfs_remove_groups(struct kobject *kobj,
				       const struct attribute_group **groups)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL_GPL(sysfs_remove_groups);

void __weak kfree_const(const void *x)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL(kfree_const);

__weak void kfree(const void *x)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL(kfree);

__weak int sysfs_create_groups(struct kobject *kobj,
            const struct attribute_group **groups)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL_GPL(sysfs_create_groups);

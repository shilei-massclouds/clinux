// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kobject.h>
#include <linux/kobject.h>
#include "../../booter/src/booter.h"

int
cl_kobject_init(void)
{
    sbi_puts("module[kobject]: init begin ...\n");
    sbi_puts("module[kobject]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_kobject_init);

void sysfs_remove_groups(struct kobject *kobj,
				       const struct attribute_group **groups)
{
    booter_panic("No impl in 'kobject'.");
}

void __weak kfree_const(const void *x)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL(kfree_const);

void kernfs_put(struct kernfs_node *kn)
{
    booter_panic("No impl in 'kobject'.");
}

void sysfs_remove_dir(struct kobject *kobj)
{
    booter_panic("No impl in 'kobject'.");
}

/*
void dump_stack(void)
{
    booter_panic("No impl 'kobject'.");
}
*/

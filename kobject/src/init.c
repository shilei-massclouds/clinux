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

__weak void kfree(const void *x)
{
    booter_panic("No impl in 'kobject'.");
}
EXPORT_SYMBOL(kfree);

__weak void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(kmem_cache_alloc);

/*
void sysfs_remove_dir(struct kobject *kobj)
{
    booter_panic("No impl in 'kobject'.");
}
*/

int sysfs_create_groups(struct kobject *kobj,
            const struct attribute_group **groups)
{
    booter_panic("No impl in 'kobject'.");
}
/*
int sysfs_create_dir_ns(struct kobject *kobj, const void *ns)
{
    booter_panic("No impl in 'kobject'.");
}
*/

struct kmem_cache *
kmalloc_caches[NR_KMALLOC_TYPES][KMALLOC_SHIFT_HIGH + 1] __ro_after_init =
{ /* initialization for https://bugs.llvm.org/show_bug.cgi?id=42570 */ };
EXPORT_SYMBOL(kmalloc_caches);

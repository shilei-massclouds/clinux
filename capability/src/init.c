// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/capability.h>
#include <linux/cred.h>
#include "../../booter/src/booter.h"

int
cl_capability_init(void)
{
    sbi_puts("module[capability]: init begin ...\n");
    sbi_puts("module[capability]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_capability_init);

int cap_capget(struct task_struct *target, kernel_cap_t *effective,
           kernel_cap_t *inheritable, kernel_cap_t *permitted)
{
    booter_panic("No impl 'slub'.");
}
int cap_capset(struct cred *new,
           const struct cred *old,
           const kernel_cap_t *effective,
           const kernel_cap_t *inheritable,
           const kernel_cap_t *permitted)
{
    booter_panic("No impl 'slub'.");
}

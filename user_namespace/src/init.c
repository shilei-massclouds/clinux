// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/key.h>
#include <linux/user_namespace.h>
#include "../../booter/src/booter.h"

int
cl_user_namespace_init(void)
{
    sbi_puts("module[user_namespace]: init begin ...\n");
    sbi_puts("module[user_namespace]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_user_namespace_init);

bool ns_capable(struct user_namespace *ns, int cap)
{
    booter_panic("No impl 'ns_capable'.");
}

void key_put(struct key *key)
{
    booter_panic("No impl 'key_put'.");
}

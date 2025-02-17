// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/user_namespace.h>
#include "../../booter/src/booter.h"

int
cl_utsname_init(void)
{
    sbi_puts("module[utsname]: init begin ...\n");
    sbi_puts("module[utsname]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_utsname_init);

// From kernel/sys.c
DECLARE_RWSEM(uts_sem);

/*
struct ucounts *inc_ucount(struct user_namespace *ns, kuid_t uid,
               enum ucount_type type)
{
    booter_panic("No impl.\n");
}
void dec_ucount(struct ucounts *ucounts, enum ucount_type type)
{
    booter_panic("No impl.\n");
}
void proc_free_inum(unsigned int inum)
{
    booter_panic("No impl.\n");
}
int proc_alloc_inum(unsigned int *inum)
{
    booter_panic("No impl.\n");
}
*/

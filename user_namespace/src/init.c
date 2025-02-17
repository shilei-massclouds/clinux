// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/highuid.h>
#include <linux/cred.h>
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

int overflowuid = DEFAULT_OVERFLOWUID;
int overflowgid = DEFAULT_OVERFLOWGID;

void retire_userns_sysctls(struct user_namespace *ns)
{
    booter_panic("No impl!\n");
}
/*
struct ucounts *inc_ucount(struct user_namespace *ns, kuid_t uid,
               enum ucount_type type)
{
    booter_panic("No impl!\n");
}
*/
bool setup_userns_sysctls(struct user_namespace *ns)
{
    booter_panic("No impl!\n");
}
/*
void proc_free_inum(unsigned int inum)
{
    booter_panic("No impl!\n");
}
*/
bool file_ns_capable(const struct file *file, struct user_namespace *ns,
             int cap)
{
    booter_panic("No impl!\n");
}
/*
void dec_ucount(struct ucounts *ucounts, enum ucount_type type)
{
    booter_panic("No impl!\n");
}
void key_free_user_ns(struct user_namespace *ns)
{
    booter_panic("No impl!\n");
}
int proc_alloc_inum(unsigned int *inum)
{
    booter_panic("No impl!\n");
}
*/
void *memdup_user_nul(const void __user *src, size_t len)
{
    booter_panic("No impl!\n");
}
bool current_chrooted(void)
{
    booter_panic("No impl!\n");
}


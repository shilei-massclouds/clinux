// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/proc_ns.h>
#include "../../booter/src/booter.h"

int
cl_pid_init(void)
{
    sbi_puts("module[pid]: init begin ...\n");
    sbi_puts("module[pid]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_pid_init);

const struct file_operations pidfd_fops;

//const struct proc_ns_operations pidns_operations;

struct pid *pidfd_pid(const struct file *file)
{
    booter_panic("No impl.\n");
}

struct file *fget_task(struct task_struct *task, unsigned int fd)
{
    booter_panic("No impl.\n");
}
int anon_inode_getfd(const char *name, const struct file_operations *fops,
             void *priv, int flags)
{
    booter_panic("No impl.\n");
}

void put_pid_ns(struct pid_namespace *ns)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(put_pid_ns);

bool ptrace_may_access(struct task_struct *task, unsigned int mode)
{
    booter_panic("No impl.\n");
}

void fput(struct file *file)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL_GPL(fput);

int __receive_fd(int fd, struct file *file, int __user *ufd, unsigned int o_flags)
{
    booter_panic("No impl.\n");
}
unsigned long __fdget(unsigned int fd)
{
    booter_panic("No impl.\n");
}


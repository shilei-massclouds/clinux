// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/tty.h>
#include <linux/sched/debug.h>
#include "../../booter/src/booter.h"

int
cl_tty_init(void)
{
    sbi_puts("module[tty]: init begin ...\n");
    sbi_puts("module[tty]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_tty_init);

ssize_t vfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    booter_panic("No impl.\n");
}
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
            const char *name)
{
    booter_panic("No impl.\n");
}
struct class *__class_create(struct module *owner, const char *name,
                 struct lock_class_key *key)
{
    booter_panic("No impl.\n");
}
int ptm_open_peer(struct file *master, struct tty_struct *tty, int flags)
{
    booter_panic("No impl.\n");
}
int is_current_pgrp_orphaned(void)
{
    booter_panic("No impl.\n");
}
int kill_pgrp(struct pid *pid, int sig, int priv)
{
    booter_panic("No impl.\n");
}
int __sched ldsem_down_write(struct ld_semaphore *sem, long timeout)
{
    booter_panic("No impl.\n");
}
void *vzalloc(unsigned long size)
{
    booter_panic("No impl.\n");
}
int iterate_fd(struct files_struct *files, unsigned n,
        int (*f)(const void *, struct file *, unsigned),
        const void *p)
{
    booter_panic("No impl.\n");
}
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
    booter_panic("No impl.\n");
}

/*
void fput(struct file *file)
{
    booter_panic("No impl.\n");
}
*/

/*
struct task_struct *pid_task(struct pid *pid, enum pid_type type)
{
    booter_panic("No impl.\n");
}
*/
struct device *device_create(struct class *class, struct device *parent,
                 dev_t devt, void *drvdata, const char *fmt, ...)
{
    booter_panic("No impl.\n");
}
struct tty_driver *console_device(int *index)
{
    booter_panic("No impl.\n");
}
void proc_tty_unregister_driver(struct tty_driver *driver)
{
    booter_panic("No impl.\n");
}

/*
int
__group_send_sig_info(int sig, struct kernel_siginfo *info, struct task_struct *p)
{
    booter_panic("No impl.\n");
}
*/

int __sched ldsem_down_read(struct ld_semaphore *sem, long timeout)
{
    booter_panic("No impl.\n");
}
int device_match_devt(struct device *dev, const void *pdevt)
{
    booter_panic("No impl.\n");
}
void __f_setown(struct file *filp, struct pid *pid, enum pid_type type,
        int force)
{
    booter_panic("No impl.\n");
}
char *__get_task_comm(char *buf, size_t buf_size, struct task_struct *tsk)
{
    booter_panic("No impl.\n");
}
int nonseekable_open(struct inode *inode, struct file *filp)
{
    booter_panic("No impl.\n");
}
void proc_tty_register_driver(struct tty_driver *driver)
{
    booter_panic("No impl.\n");
}
void ldsem_up_write(struct ld_semaphore *sem)
{
    booter_panic("No impl.\n");
}
struct device *device_create_with_groups(struct class *class,
                     struct device *parent, dev_t devt,
                     void *drvdata,
                     const struct attribute_group **groups,
                     const char *fmt, ...)
{
    booter_panic("No impl.\n");
}
int group_send_sig_info(int sig, struct kernel_siginfo *info,
            struct task_struct *p, enum pid_type type)
{
    booter_panic("No impl.\n");
}
void vfree(const void *addr)
{
    booter_panic("No impl.\n");
}
void unregister_chrdev_region(dev_t from, unsigned count)
{
    booter_panic("No impl.\n");
}
void device_destroy(struct class *class, dev_t devt)
{
    booter_panic("No impl.\n");
}
bool cancel_work_sync(struct work_struct *work)
{
    booter_panic("No impl.\n");
}
loff_t no_llseek(struct file *file, loff_t offset, int whence)
{
    booter_panic("No impl.\n");
}
int ldsem_down_read_trylock(struct ld_semaphore *sem)
{
    booter_panic("No impl.\n");
}
void ldsem_up_read(struct ld_semaphore *sem)
{
    booter_panic("No impl.\n");
}
int register_chrdev_region(dev_t from, unsigned count, const char *name)
{
    booter_panic("No impl.\n");
}
struct ctl_table_header *register_sysctl_table(struct ctl_table *table)
{
    booter_panic("No impl.\n");
}
int __init vty_init(const struct file_operations *console_fops)
{
    booter_panic("No impl.\n");
}
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
    booter_panic("No impl.\n");
}
struct device *class_find_device(struct class *class, struct device *start,
                 const void *data,
                 int (*match)(struct device *, const void *))
{
    booter_panic("No impl.\n");
}
struct cdev *cdev_alloc(void)
{
    booter_panic("No impl.\n");
}
void cdev_del(struct cdev *p)
{
    booter_panic("No impl.\n");
}
void __init_ldsem(struct ld_semaphore *sem, const char *name,
          struct lock_class_key *key)
{
    booter_panic("No impl.\n");
}

struct tty_driver *console_driver;

int fg_console;
EXPORT_SYMBOL(fg_console);

// From kernel/printk/printk.c
struct console *console_drivers;
EXPORT_SYMBOL_GPL(console_drivers);

void __weak console_lock(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_lock);

void __weak console_unlock(void)
{
    booter_panic("No impl.\n");
}
EXPORT_SYMBOL(console_unlock);

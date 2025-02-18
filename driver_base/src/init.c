// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/fwnode.h>
#include "../../booter/src/booter.h"

int
cl_driver_base_init(void)
{
    sbi_puts("module[driver_base]: init begin ...\n");
    sbi_puts("module[driver_base]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_driver_base_init);

struct device_type part_type;


void unlock_device_hotplug(void)
{
    booter_panic("No impl 'driver_base'.");
}

void device_remove_groups(struct device *dev,
              const struct attribute_group **groups)
{
    booter_panic("No impl 'driver_base'.");
}

/*
void call_srcu(struct srcu_struct *ssp, struct rcu_head *rhp,
           rcu_callback_t func)
{
    booter_panic("No impl 'driver_base'.");
}
*/

void klist_del(struct klist_node *n)
{
    booter_panic("No impl 'driver_base'.");
}

/*
int blocking_notifier_call_chain(struct blocking_notifier_head *nh,
        unsigned long val, void *v)
{
    booter_panic("No impl 'driver_base'.");
}
*/

int device_online(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
void _dev_err(const struct device *dev, const char *fmt, ...)
{
    booter_panic("No impl 'driver_base'.");
}
void bus_remove_device(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
void driver_deferred_probe_del(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}

bool kill_device(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
/*
void kobject_del(struct kobject *kobj)
{
    booter_panic("No impl 'driver_base'.");
}
*/
int devtmpfs_delete_node(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
int device_offline(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
bool refcount_dec_not_one(refcount_t *r)
{
    booter_panic("No impl 'driver_base'.");
}
int lock_device_hotplug_sysfs(void)
{
    booter_panic("No impl 'driver_base'.");
}
/*
void __srcu_read_unlock(struct srcu_struct *ssp, int idx)
{
    booter_panic("No impl 'driver_base'.");
}
*/
int kobject_synth_uevent(struct kobject *kobj, const char *buf, size_t count)
{
    booter_panic("No impl 'driver_base'.");
}

void sysfs_delete_link(struct kobject *kobj, struct kobject *targ,
            const char *name)
{
    booter_panic("No impl 'driver_base'.");
}

void device_remove_file(struct device *dev,
            const struct device_attribute *attr)
{
    booter_panic("No impl 'driver_base'.");
}

//const struct sysfs_ops kobj_sysfs_ops;

void kset_unregister(struct kset *k)
{
    booter_panic("No impl 'driver_base'.");
}
bool of_dma_is_coherent(struct device_node *np)
{
    booter_panic("No impl 'driver_base'.");
}
/*
int sysfs_create_link(struct kobject *kobj, struct kobject *target,
              const char *name)
{
    booter_panic("No impl 'driver_base'.");
}
*/
int kobject_init_and_add(struct kobject *kobj, struct kobj_type *ktype,
             struct kobject *parent, const char *fmt, ...)
{
    booter_panic("No impl 'driver_base'.");
}
struct kset *kset_create_and_add(const char *name,
                 const struct kset_uevent_ops *uevent_ops,
                 struct kobject *parent_kobj)
{
    booter_panic("No impl 'driver_base'.");
}

int __weak of_irq_get(struct device_node *dev, int index)
{
    booter_panic("No impl 'driver_base'.");
}

bool __weak is_of_node(const struct fwnode_handle *fwnode)
{
    booter_panic("No impl 'driver_base'.");
}

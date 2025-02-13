// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
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
/*
void dpm_sysfs_remove(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}

void srcu_drive_gp(struct work_struct *wp)
{
    booter_panic("No impl 'driver_base'.");
}
*/

int sysfs_create_file_ns(struct kobject *kobj, const struct attribute *attr,
             const void *ns)
{
    booter_panic("No impl 'driver_base'.");
}

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

int software_node_notify(struct device *dev, unsigned long action)
{
    booter_panic("No impl 'driver_base'.");
}

void klist_del(struct klist_node *n)
{
    booter_panic("No impl 'driver_base'.");
}

int blocking_notifier_call_chain(struct blocking_notifier_head *nh,
        unsigned long val, void *v)
{
    booter_panic("No impl 'driver_base'.");
}

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
void device_remove_properties(struct device *dev)
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


// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/fwnode.h>
#include <linux/init.h>
#include <linux/memory.h>
#include <linux/of.h>

#include "base.h"
#include "../../booter/src/booter.h"

int
cl_driver_base_init(void)
{
    sbi_puts("module[driver_base]: init begin ...\n");
    sbi_puts("module[driver_base]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_driver_base_init);

/**
 * driver_init - initialize driver model.
 *
 * Call the driver model init functions to initialize their
 * subsystems. Called early from init/main.c.
 */
void __init driver_init(void)
{
    printk("%s: ==========\n", __func__);
    /* These are the core pieces */
    devtmpfs_init();
    devices_init();
    //buses_init();
    //classes_init();
    //firmware_init();
    //hypervisor_init();

    ///* These are also core pieces, but must come after the
    // * core core pieces.
    // */
    //of_core_init();
    //platform_bus_init();
    //cpu_dev_init();
    //memory_dev_init();
    //container_dev_init();
    printk("%s: ==========\n", __func__);
}
EXPORT_SYMBOL(driver_init);

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
int device_offline(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
int lock_device_hotplug_sysfs(void)
{
    booter_panic("No impl 'driver_base'.");
}
int kobject_synth_uevent(struct kobject *kobj, const char *buf, size_t count)
{
    booter_panic("No impl 'driver_base'.");
}

void device_remove_file(struct device *dev,
            const struct device_attribute *attr)
{
    booter_panic("No impl 'driver_base'.");
}

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

int __weak of_irq_get(struct device_node *dev, int index)
{
    booter_panic("No impl 'driver_base'.");
}

bool __weak is_of_node(const struct fwnode_handle *fwnode)
{
    booter_panic("No impl 'driver_base'.");
}

__weak int devtmpfs_create_node(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(devtmpfs_create_node);

__weak int devtmpfs_delete_node(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL(devtmpfs_delete_node);

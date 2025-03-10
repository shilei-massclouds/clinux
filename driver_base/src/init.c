// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/device.h>
#include <linux/fwnode.h>
#include <linux/init.h>
#include <linux/memory.h>
#include <linux/of.h>

#include "../base.h"
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
    /* These are the core pieces */
    devtmpfs_init();
    devices_init();
    buses_init();
    classes_init();
    firmware_init();
    hypervisor_init();

    /* These are also core pieces, but must come after the
     * core core pieces.
     */
    of_core_init();
    platform_bus_init();
    cpu_dev_init();
    memory_dev_init();
    container_dev_init();
}
EXPORT_SYMBOL(driver_init);

void unlock_device_hotplug(void)
{
    booter_panic("No impl 'driver_base'.");
}

int device_online(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}

int device_offline(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
int lock_device_hotplug_sysfs(void)
{
    booter_panic("No impl 'driver_base'.");
}

bool __weak is_of_node(const struct fwnode_handle *fwnode)
{
    booter_panic("No impl 'driver_base'.");
}
EXPORT_SYMBOL_GPL(is_of_node);

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

#ifdef CONFIG_BLOCK
__weak int device_is_not_partition(struct device *dev)
{
    booter_panic("No impl 'driver_base'.");
}
#endif

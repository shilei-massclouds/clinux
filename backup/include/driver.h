// SPDX-License-Identifier: GPL-2.0
#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <klist.h>
#include <kobject.h>
#include <device.h>

struct bus_type;

struct device_driver {
    const char *name;

    struct bus_type *bus;

    const struct of_device_id *of_match_table;

    int (*probe) (struct device *dev);

    struct driver_private *p;
};

struct driver_private {
    struct kobject kobj;
    struct klist klist_devices;
    struct klist_node knode_bus;
    struct device_driver *driver;
};
#define to_driver(obj) container_of(obj, struct driver_private, kobj)

int
driver_register(struct device_driver *drv);

int bus_add_driver(struct device_driver *drv);

int
driver_attach(struct device_driver *drv);

static inline int
driver_match_device(struct device_driver *drv, struct device *dev)
{
    return drv->bus->match ? drv->bus->match(dev, drv) : 1;
}

int
driver_probe_device(struct device_driver *drv, struct device *dev);

#endif /* _DRIVER_H_ */

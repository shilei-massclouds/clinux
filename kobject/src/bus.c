// SPDX-License-Identifier: GPL-2.0
#include <klist.h>
#include <slab.h>
#include <device.h>
#include <driver.h>
#include <errno.h>
#include <export.h>
#include <printk.h>

static struct bus_type *
bus_get(struct bus_type *bus)
{
    if (bus) {
        return bus;
    }
    return NULL;
}

int
bus_add_device(struct device *dev)
{
    struct bus_type *bus = bus_get(dev->bus);
    if (bus) {
        printk("bus: '%s': add device %s\n", bus->name, dev_name(dev));
        klist_add_tail(&dev->p->knode_bus, &bus->p->klist_devices);
    }
    return 0;
}

int
bus_register(struct bus_type *bus)
{
    int retval;
    struct subsys_private *priv;

    priv = kzalloc(sizeof(struct subsys_private), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    priv->bus = bus;
    bus->p = priv;

    priv->drivers_autoprobe = 1;

    priv->drivers_kset = kset_create_and_add("drivers");
    if (!priv->drivers_kset) {
        retval = -ENOMEM;
        goto error;
    }

    klist_init(&priv->klist_devices);
    klist_init(&priv->klist_drivers);

    printk("bus: '%s': registered\n", bus->name);
    return 0;

 error:
    kfree(bus->p);
    bus->p = NULL;
    return retval;
}
EXPORT_SYMBOL(bus_register);

int
bus_add_driver(struct device_driver *drv)
{
    struct bus_type *bus;
    struct driver_private *priv;
    int error = 0;

    bus = bus_get(drv->bus);
    if (!bus)
        return -EINVAL;

    printk("bus: '%s': add driver %s\n", bus->name, drv->name);

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        panic("no memory!");
    }

    klist_init(&priv->klist_devices);
    priv->driver = drv;
    drv->p = priv;
    priv->kobj.kset = bus->p->drivers_kset;
    error = kobject_init_and_add(&priv->kobj, "%s", drv->name);
    if (error)
        panic("can not init driver private!");

    klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);
    if (drv->bus->p->drivers_autoprobe) {
        error = driver_attach(drv);
        if (error)
            panic("can not attach driver!");
    }

    return 0;
}
EXPORT_SYMBOL(bus_add_driver);

/**
 * bus_probe_device - probe drivers for a new device
 * @dev: device to probe
 *
 * - Automatically probe for a driver if the bus allows it.
 */
void
bus_probe_device(struct device *dev)
{
    struct bus_type *bus = dev->bus;

    if (!bus)
        return;

    if (bus->p->drivers_autoprobe)
        device_initial_probe(dev);
}

static struct device *
next_device(struct klist_iter *i)
{
    struct device_private *dev_prv;
    struct device *dev = NULL;
    struct klist_node *n = klist_next(i);

    if (n) {
        dev_prv = to_device_private_bus(n);
        dev = dev_prv->device;
    }
    return dev;
}

int
bus_for_each_dev(struct bus_type *bus,
                 struct device *start,
                 void *data,
                 int (*fn)(struct device *, void *))
{
    struct klist_iter i;
    struct device *dev;
    int error = 0;

    if (!bus || !bus->p)
        return -EINVAL;

    klist_iter_init_node(&bus->p->klist_devices,
                         &i,
                         (start ? &start->p->knode_bus : NULL));
    while (!error && (dev = next_device(&i)))
        error = fn(dev, data);
    klist_iter_exit(&i);
    return error;
}
EXPORT_SYMBOL(bus_for_each_dev);

static struct device_driver *
next_driver(struct klist_iter *i)
{
    struct klist_node *n = klist_next(i);
    struct driver_private *drv_priv;

    if (n) {
        drv_priv = container_of(n, struct driver_private, knode_bus);
        return drv_priv->driver;
    }
    return NULL;
}

int
bus_for_each_drv(struct bus_type *bus,
                 struct device_driver *start,
                 void *data,
                 int (*fn)(struct device_driver *, void *))
{
    struct klist_iter i;
    struct device_driver *drv;
    int error = 0;

    if (!bus)
        return -EINVAL;

    klist_iter_init_node(&bus->p->klist_drivers, &i,
                         start ? &start->p->knode_bus : NULL);
    while ((drv = next_driver(&i)) && !error)
        error = fn(drv, data);
    klist_iter_exit(&i);
    return error;
}
EXPORT_SYMBOL(bus_for_each_drv);

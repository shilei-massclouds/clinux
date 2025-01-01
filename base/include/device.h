// SPDX-License-Identifier: GPL-2.0

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <gfp.h>
#include <klist.h>
#include <ioport.h>
#include <string.h>
#include <kobject.h>

struct class;
struct device;
struct device_driver;

struct bus_type {
    const char      *name;
    const char      *dev_name;
    struct device   *dev_root;

    int (*match)(struct device *dev, struct device_driver *drv);
    int (*probe)(struct device *dev);

    struct subsys_private *p;
};

struct subsys_private {
    struct kset *drivers_kset;
    struct klist klist_devices;
    struct klist klist_drivers;
    unsigned int drivers_autoprobe:1;
    struct bus_type *bus;
    struct class *class;
};

struct device_private {
    struct klist_node knode_driver;
    struct klist_node knode_bus;
    struct klist_node knode_class;
    struct device *device;
};

#define to_device_private_driver(obj)   \
    container_of(obj, struct device_private, knode_driver)
#define to_device_private_bus(obj)      \
    container_of(obj, struct device_private, knode_bus)
#define to_device_private_class(obj)    \
    container_of(obj, struct device_private, knode_class)

struct device_type {
    const char *name;
};

struct device {
    struct kobject kobj;

    struct device *parent;

    struct device_private *p;

    const char *init_name; /* initial name of the device */

    const struct device_type *type;

    struct bus_type *bus;

    void *platform_data;

    struct device_node *of_node; /* associated device tree node */

    struct list_head devres_head;

    struct class *class;

    struct device_driver *driver;

    /* Driver data, set and get with dev_set_drvdata/dev_get_drvdata */
    void *driver_data;

    dev_t devt; /* dev_t, creates the sysfs "dev" */
};

extern int device_add(struct device *dev);

extern void put_device(struct device *dev);

extern int bus_add_device(struct device *dev);

static inline const char *
dev_name(const struct device *dev)
{
    /* Use the init name until the kobject becomes available */
    if (dev->init_name)
        return dev->init_name;

    return kobject_name(&dev->kobj);
}

extern int bus_register(struct bus_type *bus);

int dev_set_name(struct device *dev, const char *fmt, ...);

void
device_initialize(struct device *dev);

int
device_register(struct device *dev);

static inline void
dev_set_drvdata(struct device *dev, void *data)
{
    dev->driver_data = data;
}

void
bus_probe_device(struct device *dev);

void
device_initial_probe(struct device *dev);

int
bus_for_each_dev(struct bus_type *bus,
                 struct device *start,
                 void *data,
                 int (*fn)(struct device *, void *));

int
bus_for_each_drv(struct bus_type *bus,
                 struct device_driver *start,
                 void *data,
                 int (*fn)(struct device_driver *, void *));

static inline struct device *kobj_to_dev(struct kobject *kobj)
{
    return container_of(kobj, struct device, kobj);
}

struct device *get_device(struct device *dev);

#endif /* _DEVICE_H_ */

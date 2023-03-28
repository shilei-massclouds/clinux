// SPDX-License-Identifier: GPL-2.0
#include <slab.h>
#include <acgcc.h>
#include <class.h>
#include <errno.h>
#include <device.h>
#include <printk.h>
#include <export.h>
#include <device.h>
#include <driver.h>

static int
device_private_init(struct device *dev)
{
    dev->p = kzalloc(sizeof(*dev->p), GFP_KERNEL);
    if (!dev->p)
        return -ENOMEM;
    dev->p->device = dev;
    return 0;
}

int device_add(struct device *dev)
{
    int error = -EINVAL;

    printk("device: '%s': %s\n", dev_name(dev), __func__);

    if (!dev->p) {
        error = device_private_init(dev);
        if (error)
            return error;
    }

    error = bus_add_device(dev);
    if (error)
        return error;

    bus_probe_device(dev);

    if (dev->class) {
        /* tie the class to the device */
        klist_add_tail(&dev->p->knode_class,
                       &dev->class->p->klist_devices);
    }

    return 0;
}
EXPORT_SYMBOL(device_add);

/**
 * put_device - decrement reference count.
 * @dev: device in question.
 */
void put_device(struct device *dev)
{
    /* Todo: add kobject_put */
}
EXPORT_SYMBOL(put_device);

/**
 * dev_set_name - set a device name
 * @dev: device
 * @fmt: format string for the device's name
 */
int
dev_set_name(struct device *dev, const char *fmt, ...)
{
    va_list vargs;
    int err;

    va_start(vargs, fmt);
    err = kobject_set_name_vargs(&dev->kobj, fmt, vargs);
    va_end(vargs);
    return err;
}
EXPORT_SYMBOL(dev_set_name);

void
device_initialize(struct device *dev)
{
    kobject_init(&dev->kobj);
    INIT_LIST_HEAD(&dev->devres_head);
}
EXPORT_SYMBOL(device_initialize);

int
device_register(struct device *dev)
{
    device_initialize(dev);
    return device_add(dev);
}
EXPORT_SYMBOL(device_register);

static int
__device_attach_driver(struct device_driver *drv, void *_data)
{
    int ret;
    struct device *dev = _data;

    ret = driver_match_device(drv, dev);
    if (ret == 0) {
        /* no match */
        return 0;
    } else if (ret == -EPROBE_DEFER) {
        panic("Device match requests probe deferral");
    } else if (ret < 0) {
        panic("Bus failed to match device: %d", ret);
        return ret;
    } /* ret > 0 means positive match */

    panic("%s", __func__);
    return driver_probe_device(drv, dev);
}

static int
__device_attach(struct device *dev)
{
    BUG_ON(dev->driver);
    return bus_for_each_drv(dev->bus, NULL, dev, __device_attach_driver);
}

void
device_initial_probe(struct device *dev)
{
    __device_attach(dev);
}

struct device *
get_device(struct device *dev)
{
    return dev ? kobj_to_dev(&dev->kobj) : NULL;
}
EXPORT_SYMBOL(get_device);

// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <export.h>
#include <printk.h>
#include <virtio.h>

int _virtio_index = 0;
EXPORT_SYMBOL(_virtio_index);

int
virtio_finalize_features(struct virtio_device *dev)
{
    unsigned status;

    int ret = dev->config->finalize_features(dev);
    if (ret)
        return ret;

    if (!virtio_has_feature(dev, VIRTIO_F_VERSION_1))
        return 0;

    virtio_add_status(dev, VIRTIO_CONFIG_S_FEATURES_OK);
    status = dev->config->get_status(dev);
    if (!(status & VIRTIO_CONFIG_S_FEATURES_OK)) {
        panic("virtio: device refuses features: %x\n", status);
        return -ENODEV;
    }
    return 0;
}
EXPORT_SYMBOL(virtio_finalize_features);

static void
__virtio_config_changed(struct virtio_device *dev)
{
    struct virtio_driver *drv = drv_to_virtio(dev->dev.driver);

    if (!dev->config_enabled)
        dev->config_change_pending = true;
}

void
virtio_config_enable(struct virtio_device *dev)
{
    dev->config_enabled = true;
    if (dev->config_change_pending)
        __virtio_config_changed(dev);
    dev->config_change_pending = false;
}
EXPORT_SYMBOL(virtio_config_enable);

static int
virtio_dev_probe(struct device *_d)
{
    int i;
    int err;
    u64 device_features;
    u64 driver_features;
    u64 driver_features_legacy;
    struct virtio_device *dev = dev_to_virtio(_d);
    struct virtio_driver *drv = drv_to_virtio(dev->dev.driver);

    /* We have a driver! */
    virtio_add_status(dev, VIRTIO_CONFIG_S_DRIVER);

    /* Figure out what features the device supports. */
    device_features = dev->config->get_features(dev);

    /* Figure out what features the driver supports. */
    driver_features = 0;
    for (i = 0; i < drv->feature_table_size; i++) {
        unsigned int f = drv->feature_table[i];
        BUG_ON(f >= 64);
        driver_features |= (1ULL << f);
    }

    /* Some drivers have a separate feature table for virtio v1.0 */
    if (drv->feature_table_legacy) {
        driver_features_legacy = 0;
        for (i = 0; i < drv->feature_table_size_legacy; i++) {
            unsigned int f = drv->feature_table_legacy[i];
            BUG_ON(f >= 64);
            driver_features_legacy |= (1ULL << f);
        }
    } else {
        driver_features_legacy = driver_features;
    }

    if (device_features & (1ULL << VIRTIO_F_VERSION_1))
        dev->features = driver_features & device_features;
    else
        dev->features = driver_features_legacy & device_features;

    /* Transport features always preserved to pass to finalize_features. */
    for (i = VIRTIO_TRANSPORT_F_START; i < VIRTIO_TRANSPORT_F_END; i++)
        if (device_features & (1ULL << i))
            __virtio_set_bit(dev, i);

    if (drv->validate) {
        panic("NOW: no validate!");
        err = drv->validate(dev);
        if (err)
            goto err;
    }

    err = virtio_finalize_features(dev);
    if (err)
        goto err;

    err = drv->probe(dev);
    if (err)
        goto err;

    /* If probe didn't do it, mark device DRIVER_OK ourselves. */
    if (!(dev->config->get_status(dev) & VIRTIO_CONFIG_S_DRIVER_OK))
        virtio_device_ready(dev);

    if (drv->scan) {
        panic("NOW: no scan!");
        drv->scan(dev);
    }

    virtio_config_enable(dev);
    return 0;
 err:
    virtio_add_status(dev, VIRTIO_CONFIG_S_FAILED);
    return err;
}

static inline int
virtio_id_match(const struct virtio_device *dev,
                const struct virtio_device_id *id)
{
    if (id->device != dev->id.device && id->device != VIRTIO_DEV_ANY_ID)
        return 0;

    return id->vendor == VIRTIO_DEV_ANY_ID || id->vendor == dev->id.vendor;
}

static int
virtio_dev_match(struct device *_dv, struct device_driver *_dr)
{
    unsigned int i;
    struct virtio_device *dev = dev_to_virtio(_dv);
    const struct virtio_device_id *ids;

    ids = drv_to_virtio(_dr)->id_table;
    for (i = 0; ids[i].device; i++)
        if (virtio_id_match(dev, &ids[i]))
            return 1;
    return 0;
}

static struct bus_type virtio_bus = {
    .name  = "virtio",
    .match = virtio_dev_match,
    .probe = virtio_dev_probe,
};

void
virtio_add_status(struct virtio_device *dev, unsigned int status)
{
    dev->config->set_status(dev, dev->config->get_status(dev) | status);
}
EXPORT_SYMBOL(virtio_add_status);

/**
 * register_virtio_device - register virtio device
 * @dev: virtio device to be registered
 *
 * On error, the caller must call put_device
 * on &@dev->dev (and not kfree),
 * as another code path may have obtained a reference to @dev.
 *
 * Returns: 0 on suceess, -error on failure
 */
int
register_virtio_device(struct virtio_device *dev)
{
    int err;

    dev->dev.bus = &virtio_bus;
    device_initialize(&dev->dev);

    dev->index = _virtio_index++;
    dev_set_name(&dev->dev, "virtio%u", dev->index);

    dev->config_enabled = false;
    dev->config_change_pending = false;

    /* We always start by resetting the device, in case a previous
     * driver messed it up.  This also tests that code path a little. */
    dev->config->reset(dev);

    /* Acknowledge that we've seen the device. */
    virtio_add_status(dev, VIRTIO_CONFIG_S_ACKNOWLEDGE);

    INIT_LIST_HEAD(&dev->vqs);

    /*
     * device_add() causes the bus infrastructure to look for a matching
     * driver.
     */
    err = device_add(&dev->dev);
    if (err) {
        virtio_add_status(dev, VIRTIO_CONFIG_S_FAILED);
        panic("can not add device!");
    }
    return err;
}
EXPORT_SYMBOL(register_virtio_device);

int
register_virtio_driver(struct virtio_driver *driver)
{
    /* Catch this early. */
    BUG_ON(driver->feature_table_size && !driver->feature_table);
    driver->driver.bus = &virtio_bus;
    return driver_register(&driver->driver);
}
EXPORT_SYMBOL(register_virtio_driver);

static int
virtio_init(void)
{
    if (bus_register(&virtio_bus) != 0)
        panic("virtio bus registration failed");
    return 0;
}

void
virtio_check_driver_offered_feature(const struct virtio_device *vdev,
                                    unsigned int fbit)
{
    unsigned int i;
    struct virtio_driver *drv = drv_to_virtio(vdev->dev.driver);

    for (i = 0; i < drv->feature_table_size; i++)
        if (drv->feature_table[i] == fbit)
            return;

    if (drv->feature_table_legacy) {
        for (i = 0; i < drv->feature_table_size_legacy; i++)
            if (drv->feature_table_legacy[i] == fbit)
                return;
    }

    BUG();
}
EXPORT_SYMBOL(virtio_check_driver_offered_feature);

int
init_module(void)
{
    printk("module[virtio]: init begin ...\n");
    virtio_init();
    printk("module[virtio]: init end!\n");
    return 0;
}

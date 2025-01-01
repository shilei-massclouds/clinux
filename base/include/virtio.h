/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_VIRTIO_H
#define _LINUX_VIRTIO_H

#include <types.h>
#include <device.h>
#include <driver.h>
#include <virtio_config.h>

typedef __u16 __virtio16;
typedef __u32 __virtio32;
typedef __u64 __virtio64;

extern int _virtio_index;

#define VIRTIO_ID_BLOCK     2 /* virtio block */

struct virtio_device_id {
    u32 device;
    u32 vendor;
};
#define VIRTIO_DEV_ANY_ID   0xffffffff

struct virtio_device {
    int index;
    struct device dev;
    bool config_enabled;
    bool config_change_pending;
    struct virtio_device_id id;
    const struct virtio_config_ops *config;
    struct list_head vqs;
    u64 features;
    void *priv;
};

/**
 * virtqueue - a queue to register buffers for sending or receiving.
 * @list: the chain of virtqueues for this device
 * @callback: the function to call when buffers are consumed (can be NULL).
 * @name: the name of this virtqueue (mainly for debugging)
 * @vdev: the virtio device this queue was created for.
 * @priv: a pointer for the virtqueue implementation to use.
 * @index: the zero-based ordinal number for this queue.
 * @num_free: number of elements we expect to be able to fit.
 *
 * A note on @num_free: with indirect buffers, each buffer needs one
 * element in the queue, otherwise a buffer will need one element per
 * sg element.
 */
struct virtqueue {
    struct list_head list;
    void (*callback)(struct virtqueue *vq);
    const char *name;
    struct virtio_device *vdev;
    unsigned int index;
    unsigned int num_free;
    void *priv;
};

struct virtio_driver {
    struct device_driver driver;
    const struct virtio_device_id *id_table;
    const unsigned int *feature_table;
    unsigned int feature_table_size;
    const unsigned int *feature_table_legacy;
    unsigned int feature_table_size_legacy;
    int (*validate)(struct virtio_device *dev);
    int (*probe)(struct virtio_device *dev);
    void (*scan)(struct virtio_device *dev);
    /*
    void (*remove)(struct virtio_device *dev);
    void (*config_changed)(struct virtio_device *dev);
    */
};

int
register_virtio_device(struct virtio_device *dev);

int
register_virtio_driver(struct virtio_driver *driver);

static inline struct virtio_device *dev_to_virtio(struct device *_dev)
{
    return container_of(_dev, struct virtio_device, dev);
}

static inline struct virtio_driver *drv_to_virtio(struct device_driver *drv)
{
    return container_of(drv, struct virtio_driver, driver);
}

void
virtio_add_status(struct virtio_device *dev, unsigned int status);

/**
 * __virtio_set_bit - helper to set feature bits. For use by transports.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline void
__virtio_set_bit(struct virtio_device *vdev, unsigned int fbit)
{
    /* Did you forget to fix assumptions on max features? */
    if (__builtin_constant_p(fbit))
        BUG_ON(fbit >= 64);
    else
        BUG_ON(fbit >= 64);

    vdev->features |= BIT_ULL(fbit);
}

/**
 * __virtio_clear_bit - helper to clear feature bits. For use by transports.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline void __virtio_clear_bit(struct virtio_device *vdev,
                      unsigned int fbit)
{
    /* Did you forget to fix assumptions on max features? */
    if (__builtin_constant_p(fbit))
        BUG_ON(fbit >= 64);
    else
        BUG_ON(fbit >= 64);

    vdev->features &= ~BIT_ULL(fbit);
}

void
virtio_check_driver_offered_feature(const struct virtio_device *vdev,
                                    unsigned int fbit);

/**
 * __virtio_test_bit - helper to test feature bits. For use by transports.
 *                     Devices should normally use virtio_has_feature,
 *                     which includes more checks.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline bool
__virtio_test_bit(const struct virtio_device *vdev, unsigned int fbit)
{
    /* Did you forget to fix assumptions on max features? */
    if (__builtin_constant_p(fbit))
        BUG_ON(fbit >= 64);
    else
        BUG_ON(fbit >= 64);

    return vdev->features & BIT_ULL(fbit);
}

/**
 * virtio_has_feature - helper to determine if this device has this feature.
 * @vdev: the device
 * @fbit: the feature bit
 */
static inline bool
virtio_has_feature(const struct virtio_device *vdev, unsigned int fbit)
{
    if (fbit < VIRTIO_TRANSPORT_F_START)
        virtio_check_driver_offered_feature(vdev, fbit);

    return __virtio_test_bit(vdev, fbit);
}

/**
 * virtio_device_ready - enable vq use in probe function
 * @vdev: the device
 *
 * Driver must call this to use vqs in the probe function.
 *
 * Note: vqs are enabled automatically after probe returns.
 */
static inline void
virtio_device_ready(struct virtio_device *dev)
{
    unsigned status = dev->config->get_status(dev);

    BUG_ON(status & VIRTIO_CONFIG_S_DRIVER_OK);
    dev->config->set_status(dev, status | VIRTIO_CONFIG_S_DRIVER_OK);
}

/* Read @count fields, @bytes each. */
static inline void
__virtio_cread_many(struct virtio_device *vdev, unsigned int offset,
                    void *buf, size_t count, size_t bytes)
{
    int i;

    for (i = 0; i < count; i++)
        vdev->config->get(vdev, offset + bytes * i, buf + i * bytes, bytes);
}

static inline int
virtio_find_vqs(struct virtio_device *vdev,
                unsigned nvqs, struct virtqueue *vqs[],
                vq_callback_t *callbacks[],
                const char * const names[],
                struct irq_affinity *desc)
{
    return vdev->config->find_vqs(vdev, nvqs, vqs,
                                  callbacks, names, NULL, desc);
}

#endif /* _LINUX_VIRTIO_H */

/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_VIRTIO_CONFIG_H
#define _LINUX_VIRTIO_CONFIG_H

#include <interrupt.h>

/* Status byte for guest to report progress, and synchronize features. */
/* We have seen device and processed generic fields (VIRTIO_CONFIG_F_VIRTIO) */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
/* We have found a driver for the device. */
#define VIRTIO_CONFIG_S_DRIVER      2
/* Driver has used its parts of the config, and is happy */
#define VIRTIO_CONFIG_S_DRIVER_OK   4
/* Driver has finished configuring features */
#define VIRTIO_CONFIG_S_FEATURES_OK 8

/* We've given up on this device. */
#define VIRTIO_CONFIG_S_FAILED      0x80

#define VIRTIO_TRANSPORT_F_START    28
#define VIRTIO_TRANSPORT_F_END      38

#define VIRTIO_F_VERSION_1      32

struct virtqueue;
struct virtio_device;

typedef void vq_callback_t(struct virtqueue *);
struct virtio_config_ops {
    void (*get)(struct virtio_device *vdev, unsigned offset,
                void *buf, unsigned len);
    void (*set)(struct virtio_device *vdev, unsigned offset,
                const void *buf, unsigned len);
    u32 (*generation)(struct virtio_device *vdev);
    u8 (*get_status)(struct virtio_device *vdev);
    void (*set_status)(struct virtio_device *vdev, u8 status);
    void (*reset)(struct virtio_device *vdev);
    int (*find_vqs)(struct virtio_device *, unsigned nvqs,
                    struct virtqueue *vqs[], vq_callback_t *callbacks[],
                    const char * const names[], const bool *ctx,
                    struct irq_affinity *desc);
    void (*del_vqs)(struct virtio_device *);
    u64 (*get_features)(struct virtio_device *vdev);
    int (*finalize_features)(struct virtio_device *vdev);
    const char *(*bus_name)(struct virtio_device *vdev);
    int (*set_vq_affinity)(struct virtqueue *vq,
                           const struct cpumask *cpu_mask);
    const struct cpumask *(*get_vq_affinity)(struct virtio_device *vdev,
                                             int index);
};

/* Config space accessors. */
#define virtio_cread(vdev, structname, member, ptr)         \
    do {                                \
        typeof(((structname*)0)->member) virtio_cread_v;    \
        switch (sizeof(virtio_cread_v)) {           \
        case 1:                         \
        case 2:                         \
        case 4:                         \
            vdev->config->get((vdev),   \
                              offsetof(structname, member), \
                              &virtio_cread_v, \
                              sizeof(virtio_cread_v)); \
            break;                      \
        default:                        \
            __virtio_cread_many((vdev),                       \
                                offsetof(structname, member), \
                                &virtio_cread_v,              \
                                1,                            \
                                sizeof(virtio_cread_v));      \
            break;                      \
        }                               \
        *(ptr) = virtio_cread_v;        \
    } while(0)

/* Conditional config space accessors. */
#define virtio_cread_feature(vdev, fbit, structname, member, ptr)   \
    ({                              \
        int _r = 0;                     \
        if (!virtio_has_feature(vdev, fbit))            \
            _r = -ENOENT;                   \
        else                            \
            virtio_cread((vdev), structname, member, ptr);  \
        _r;                         \
    })

#endif /* _LINUX_VIRTIO_CONFIG_H */

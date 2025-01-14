// SPDX-License-Identifier: GPL-2.0+

#ifndef _UAPI_LINUX_VIRTIO_RING_H
#define _UAPI_LINUX_VIRTIO_RING_H

#include <scatterlist.h>

/* This marks a buffer as continuing via the next field. */
#define VRING_DESC_F_NEXT       1
/* This marks a buffer as write-only (otherwise read-only). */
#define VRING_DESC_F_WRITE      2
/* This means the buffer contains a list of buffer descriptors. */
#define VRING_DESC_F_INDIRECT   4

/* We support indirect buffer descriptors */
#define VIRTIO_RING_F_INDIRECT_DESC 28

/* The Guest publishes the used index for which it expects an interrupt
 * at the end of the avail ring. Host should ignore the avail->flags field. */
/* The Host publishes the avail index for which it expects a kick
 * at the end of the used ring. Guest should ignore the used->flags field. */
#define VIRTIO_RING_F_EVENT_IDX     29

/*
 * If clear - device has the platform DMA (e.g. IOMMU) bypass quirk feature.
 * If set - use platform DMA tools to access the memory.
 *
 * Note the reverse polarity (compared to most other features),
 * this is for compatibility with legacy systems.
 */
#define VIRTIO_F_ACCESS_PLATFORM    33

/* This feature indicates support for the packed virtqueue layout. */
#define VIRTIO_F_RING_PACKED        34

/*
 * This feature indicates that memory accesses by the driver and the
 * device are ordered in a way described by the platform.
 */
#define VIRTIO_F_ORDER_PLATFORM     36

#define VRING_AVAIL_ALIGN_SIZE 2
#define VRING_USED_ALIGN_SIZE 4
#define VRING_DESC_ALIGN_SIZE 16

/* The Guest uses this in avail->flags to advise the Host: don't interrupt me
 * when you consume a buffer.  It's unreliable, so it's simply an
 * optimization.  */
#define VRING_AVAIL_F_NO_INTERRUPT  1

#define vring_used_event(vr) ((vr)->avail->ring[(vr)->num])

/* Virtio ring descriptors: 16 bytes.  These can chain together via "next". */
struct vring_desc {
    /* Address (guest-physical). */
    __virtio64 addr;
    /* Length. */
    __virtio32 len;
    /* The flags as indicated above. */
    __virtio16 flags;
    /* We chain unused descriptors via this, too */
    __virtio16 next;
};

struct vring_avail {
    __virtio16 flags;
    __virtio16 idx;
    __virtio16 ring[];
};

/* u32 is used here for ids for padding reasons. */
struct vring_used_elem {
    /* Index of start of used descriptor chain. */
    __virtio32 id;
    /* Total length of the descriptor chain which was used (written to) */
    __virtio32 len;
};

typedef struct vring_used_elem
__attribute__((aligned(VRING_USED_ALIGN_SIZE))) vring_used_elem_t;

struct vring_used {
    __virtio16 flags;
    __virtio16 idx;
    vring_used_elem_t ring[];
};

typedef struct vring_desc
__attribute__((aligned(VRING_DESC_ALIGN_SIZE))) vring_desc_t;
typedef struct vring_avail
__attribute__((aligned(VRING_AVAIL_ALIGN_SIZE))) vring_avail_t;
typedef struct vring_used
__attribute__((aligned(VRING_USED_ALIGN_SIZE))) vring_used_t;

struct vring {
    unsigned int num;

    vring_desc_t *desc;

    vring_avail_t *avail;

    vring_used_t *used;
};

void vring_transport_features(struct virtio_device *vdev);

struct virtqueue *
vring_create_virtqueue(unsigned int index,
                       unsigned int num,
                       unsigned int vring_align,
                       struct virtio_device *vdev,
                       bool weak_barriers,
                       bool may_reduce_num,
                       bool context,
                       bool (*notify)(struct virtqueue *),
                       void (*callback)(struct virtqueue *),
                       const char *name);

static inline unsigned
vring_size(unsigned int num, unsigned long align)
{
    unsigned ret;
    ret = sizeof(struct vring_desc) * num + sizeof(__virtio16) * (3 + num);
    ret = ALIGN(ret, align);
    ret += sizeof(__virtio16) * 3;
    ret += sizeof(struct vring_used_elem) * num;
    return ret;
}

static inline void
vring_init(struct vring *vr, unsigned int num, void *p, unsigned long align)
{
    vr->num = num;
    vr->desc = p;
    vr->avail = (struct vring_avail *)((char *)p + num * sizeof(struct vring_desc));
    vr->used = (void *)(ALIGN((uintptr_t)&vr->avail->ring[num] +
                              sizeof(__virtio16), align));
}

unsigned int
virtqueue_get_vring_size(struct virtqueue *_vq);

dma_addr_t
virtqueue_get_desc_addr(struct virtqueue *_vq);

int
virtqueue_add_sgs(struct virtqueue *_vq, struct scatterlist *sgs[],
                  unsigned int out_sgs, unsigned int in_sgs,
                  void *data, gfp_t gfp);

bool virtqueue_notify(struct virtqueue *_vq);

irqreturn_t vring_interrupt(int irq, void *_vq);

void virtqueue_disable_cb(struct virtqueue *_vq);

bool virtqueue_enable_cb(struct virtqueue *_vq);

void *virtqueue_get_buf(struct virtqueue *_vq, unsigned int *len);

#endif /* _UAPI_LINUX_VIRTIO_RING_H */

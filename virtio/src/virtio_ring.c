// SPDX-License-Identifier: GPL-2.0+

#include <slab.h>
#include <types.h>
#include <export.h>
#include <virtio.h>
#include <scatterlist.h>
#include <virtio_ring.h>
#include <dma-direction.h>

#define to_vvq(_vq) container_of(_vq, struct vring_virtqueue, vq)

struct vring_virtqueue {
    struct virtqueue vq;

    /* Is this a packed ring? */
    bool packed_ring;

    /* Is DMA API used? */
    bool use_dma_api;

    /* Can we use weak barriers? */
    bool weak_barriers;

    /* Other side has made a mess, don't try any more. */
    bool broken;

    /* Host supports indirect buffers */
    bool indirect;

    /* Host publishes avail event idx */
    bool event;

    /* Head of free buffer list. */
    unsigned int free_head;
    /* Number we've added since last sync. */
    unsigned int num_added;

    /* Last used index we've seen. */
    u16 last_used_idx;

    struct {
        /* Actual memory layout for this queue. */
        struct vring vring;

        /* Last written value to avail->flags */
        u16 avail_flags_shadow;

        /*
         * Last written value to avail->idx in
         * guest byte order.
         */
        u16 avail_idx_shadow;

        /* Per-descriptor state. */
        struct vring_desc_state_split *desc_state;

        /* DMA address and size information */
        dma_addr_t queue_dma_addr;
        size_t queue_size_in_bytes;
    } split;

    /* How to notify other side. FIXME: commonalize hcalls! */
    bool (*notify)(struct virtqueue *vq);

    /* DMA, allocation, and size information */
    bool we_own_ring;
};

struct vring_desc_state_split {
    void *data;                     /* Data for callback. */
    struct vring_desc *indir_desc;  /* Indirect descriptor, if any. */
};

static void
vring_free_queue(struct virtio_device *vdev, size_t size, void *queue,
                 dma_addr_t dma_handle)
{
    free_pages_exact(queue, PAGE_ALIGN(size));
}

/* Manipulates transport-specific feature bits. */
void vring_transport_features(struct virtio_device *vdev)
{
    unsigned int i;

    for (i = VIRTIO_TRANSPORT_F_START; i < VIRTIO_TRANSPORT_F_END; i++) {
        switch (i) {
        case VIRTIO_RING_F_INDIRECT_DESC:
            break;
        case VIRTIO_RING_F_EVENT_IDX:
            break;
        case VIRTIO_F_VERSION_1:
            break;
        case VIRTIO_F_ACCESS_PLATFORM:
            break;
        case VIRTIO_F_RING_PACKED:
            break;
        case VIRTIO_F_ORDER_PLATFORM:
            break;
        default:
            /* We don't understand this bit. */
            __virtio_clear_bit(vdev, i);
        }
    }
}
EXPORT_SYMBOL(vring_transport_features);

static void *
vring_alloc_queue(struct virtio_device *vdev, size_t size,
                  dma_addr_t *dma_handle, gfp_t flag)
{
    void *queue = alloc_pages_exact(PAGE_ALIGN(size), flag);

    BUG_ON(virtio_has_feature(vdev, VIRTIO_F_ACCESS_PLATFORM));

    if (queue) {
        phys_addr_t phys_addr = virt_to_phys(queue);
        *dma_handle = (dma_addr_t)phys_addr;

        BUG_ON(*dma_handle != phys_addr);
    }
    return queue;
}

/* Only available for split ring */
struct virtqueue *
__vring_new_virtqueue(unsigned int index,
                      struct vring vring,
                      struct virtio_device *vdev,
                      bool weak_barriers,
                      bool context,
                      bool (*notify)(struct virtqueue *),
                      void (*callback)(struct virtqueue *),
                      const char *name)
{
    unsigned int i;
    struct vring_virtqueue *vq;

    if (virtio_has_feature(vdev, VIRTIO_F_RING_PACKED))
        panic("unsupport ring packed!");

    vq = kmalloc(sizeof(*vq), GFP_KERNEL);
    if (!vq)
        return NULL;

    vq->packed_ring = false;
    vq->vq.callback = callback;
    vq->vq.vdev = vdev;
    vq->vq.name = name;
    vq->vq.num_free = vring.num;
    vq->vq.index = index;
    vq->we_own_ring = false;
    vq->notify = notify;
    vq->weak_barriers = weak_barriers;
    vq->broken = false;
    vq->last_used_idx = 0;
    vq->num_added = 0;
    vq->use_dma_api = false;
    list_add_tail(&vq->vq.list, &vdev->vqs);

    vq->indirect = virtio_has_feature(vdev, VIRTIO_RING_F_INDIRECT_DESC) && !context;
    vq->event = virtio_has_feature(vdev, VIRTIO_RING_F_EVENT_IDX);
    BUG_ON(!(vq->event));

    if (virtio_has_feature(vdev, VIRTIO_F_ORDER_PLATFORM))
        vq->weak_barriers = false;

    vq->split.queue_dma_addr = 0;
    vq->split.queue_size_in_bytes = 0;

    vq->split.vring = vring;
    vq->split.avail_flags_shadow = 0;
    vq->split.avail_idx_shadow = 0;

    /* No callback?  Tell other side not to bother us. */
    if (!callback) {
        vq->split.avail_flags_shadow |= VRING_AVAIL_F_NO_INTERRUPT;
    }

    vq->split.desc_state =
        kmalloc_array(vring.num, sizeof(struct vring_desc_state_split),
                      GFP_KERNEL);
    if (!vq->split.desc_state) {
        kfree(vq);
        return NULL;
    }

    /* Put everything in free lists. */
    vq->free_head = 0;
    for (i = 0; i < vring.num-1; i++)
        vq->split.vring.desc[i].next = (u16)(i + 1);
    memset(vq->split.desc_state, 0,
           vring.num * sizeof(struct vring_desc_state_split));

    return &vq->vq;
}

static struct virtqueue *
vring_create_virtqueue_split(unsigned int index,
                             unsigned int num,
                             unsigned int vring_align,
                             struct virtio_device *vdev,
                             bool weak_barriers,
                             bool may_reduce_num,
                             bool context,
                             bool (*notify)(struct virtqueue *),
                             void (*callback)(struct virtqueue *),
                             const char *name)
{
    dma_addr_t dma_addr;
    size_t queue_size_in_bytes;
    struct vring vring;
    struct virtqueue *vq;
    void *queue = NULL;

    /* We assume num is a power of 2. */
    if (num & (num - 1)) {
        panic("Bad virtqueue length %u\n", num);
        return NULL;
    }

    /* TODO: allocate each queue chunk individually */
    for (; num && vring_size(num, vring_align) > PAGE_SIZE; num /= 2) {
        queue = vring_alloc_queue(vdev, vring_size(num, vring_align),
                                  &dma_addr,
                                  GFP_KERNEL|__GFP_NOWARN|__GFP_ZERO);
        if (queue)
            break;
        if (!may_reduce_num)
            return NULL;
    }

    if (!num)
        return NULL;

    if (!queue) {
        /* Try to get a single page. You are my only hope! */
        queue = vring_alloc_queue(vdev, vring_size(num, vring_align),
                                  &dma_addr, GFP_KERNEL|__GFP_ZERO);
    }
    if (!queue)
        return NULL;

    queue_size_in_bytes = vring_size(num, vring_align);
    vring_init(&vring, num, queue, vring_align);

    vq = __vring_new_virtqueue(index, vring, vdev, weak_barriers, context,
                               notify, callback, name);
    if (!vq) {
        vring_free_queue(vdev, queue_size_in_bytes, queue,
                 dma_addr);
        return NULL;
    }

    to_vvq(vq)->split.queue_dma_addr = dma_addr;
    to_vvq(vq)->split.queue_size_in_bytes = queue_size_in_bytes;
    to_vvq(vq)->we_own_ring = true;
    return vq;
}

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
                       const char *name)
{
    return vring_create_virtqueue_split(index, num, vring_align,
                                        vdev, weak_barriers,
                                        may_reduce_num, context, notify,
                                        callback, name);
}
EXPORT_SYMBOL(vring_create_virtqueue);

unsigned int
virtqueue_get_vring_size(struct virtqueue *_vq)
{
    return to_vvq(_vq)->split.vring.num;
}
EXPORT_SYMBOL(virtqueue_get_vring_size);

dma_addr_t
virtqueue_get_desc_addr(struct virtqueue *_vq)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    BUG_ON(!vq->we_own_ring);
    return vq->split.queue_dma_addr;
}
EXPORT_SYMBOL(virtqueue_get_desc_addr);

static inline bool
virtqueue_use_indirect(struct virtqueue *_vq, unsigned int total_sg)
{
    struct vring_virtqueue *vq = to_vvq(_vq);
    return (vq->indirect && total_sg > 1 && vq->vq.num_free);
}

static struct vring_desc *
alloc_indirect_split(struct virtqueue *_vq,
                     unsigned int total_sg,
                     gfp_t gfp)
{
    unsigned int i;
    struct vring_desc *desc;

    /*
     * We require lowmem mappings for the descriptors because
     * otherwise virt_to_phys will give us bogus addresses in the
     * virtqueue.
     */
    gfp &= ~__GFP_HIGHMEM;

    desc = kmalloc_array(total_sg, sizeof(struct vring_desc), gfp);
    if (!desc)
        panic("out of memory!");

    for (i = 0; i < total_sg; i++)
        desc[i].next = i + 1;
    return desc;
}

/* Map one sg entry. */
static dma_addr_t
vring_map_one_sg(const struct vring_virtqueue *vq,
                 struct scatterlist *sg,
                 enum dma_data_direction direction)
{
    return (dma_addr_t)sg_phys(sg);
}

static dma_addr_t
vring_map_single(const struct vring_virtqueue *vq,
                 void *cpu_addr, size_t size,
                 enum dma_data_direction direction)
{
    return (dma_addr_t)virt_to_phys(cpu_addr);
}

/**
 * virtqueue_notify - second half of split virtqueue_kick call.
 * @_vq: the struct virtqueue
 *
 * This does not need to be serialized.
 *
 * Returns false if host notify failed or queue is broken, otherwise true.
 */
bool virtqueue_notify(struct virtqueue *_vq)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    if (unlikely(vq->broken))
        return false;

    /* Prod other side to tell it about changes. */
    if (!vq->notify(_vq)) {
        vq->broken = true;
        return false;
    }
    return true;
}
EXPORT_SYMBOL(virtqueue_notify);

bool virtqueue_kick(struct virtqueue *vq)
{
    return virtqueue_notify(vq);
}
EXPORT_SYMBOL(virtqueue_kick);

static inline int
virtqueue_add_split(struct virtqueue *_vq,
                    struct scatterlist *sgs[],
                    unsigned int total_sg,
                    unsigned int out_sgs,
                    unsigned int in_sgs,
                    void *data,
                    void *ctx,
                    gfp_t gfp)
{
    int head;
    bool indirect;
    struct scatterlist *sg;
    struct vring_desc *desc;
    unsigned int i, n, avail, descs_used, prev, err_idx;
    struct vring_virtqueue *vq = to_vvq(_vq);

    BUG_ON(!virtqueue_use_indirect(_vq, total_sg));

    head = vq->free_head;

    desc = alloc_indirect_split(_vq, total_sg, gfp);
    if (desc) {
        /* Use a single buffer which doesn't continue */
        indirect = true;
        /* Set up rest to use this indirect table. */
        i = 0;
        descs_used = 1;
    } else {
        panic("no desc!");
    }

    for (n = 0; n < out_sgs; n++) {
        for (sg = sgs[n]; sg; sg = sg_next(sg)) {
            dma_addr_t addr = vring_map_one_sg(vq, sg, DMA_TO_DEVICE);

            desc[i].flags = VRING_DESC_F_NEXT;
            desc[i].addr = addr;
            desc[i].len = sg->length;
            prev = i;
            i = desc[i].next;
        }
    }
    for (; n < (out_sgs + in_sgs); n++) {
        for (sg = sgs[n]; sg; sg = sg_next(sg)) {
            dma_addr_t addr = vring_map_one_sg(vq, sg, DMA_FROM_DEVICE);

            desc[i].flags = VRING_DESC_F_NEXT | VRING_DESC_F_WRITE;
            desc[i].addr = addr;
            desc[i].len = sg->length;
            prev = i;
            i = desc[i].next;
        }
    }
    /* Last one doesn't continue. */
    desc[prev].flags &= ~VRING_DESC_F_NEXT;

    if (indirect) {
        /* Now that the indirect table is filled in, map it. */
        dma_addr_t addr;
        addr = vring_map_single(vq, desc,
                                total_sg * sizeof(struct vring_desc),
                                DMA_TO_DEVICE);

        vq->split.vring.desc[head].flags = VRING_DESC_F_INDIRECT;
        vq->split.vring.desc[head].addr = addr;
        vq->split.vring.desc[head].len =
            total_sg * sizeof(struct vring_desc);
    }

    /* We're using some buffers from the free list. */
    vq->vq.num_free -= descs_used;

    /* Update free pointer */
    if (indirect)
        vq->free_head = vq->split.vring.desc[head].next;
    else
        vq->free_head = i;

    /* Store token and indirect buffer state. */
    vq->split.desc_state[head].data = data;
    if (indirect)
        vq->split.desc_state[head].indir_desc = desc;
    else
        vq->split.desc_state[head].indir_desc = ctx;

    /* Put entry in available array
     * (but don't update avail->idx until they do sync). */
    avail = vq->split.avail_idx_shadow & (vq->split.vring.num - 1);
    vq->split.vring.avail->ring[avail] = head;

    /* Descriptors and available array need to be set before we expose the
     * new available array entries. */
    vq->split.avail_idx_shadow++;
    vq->split.vring.avail->idx = vq->split.avail_idx_shadow;
    vq->num_added++;

    /* This is very unlikely, but theoretically possible.  Kick
     * just in case. */
    if (unlikely(vq->num_added == (1 << 16) - 1))
        virtqueue_kick(_vq);

    return 0;
}

static inline int
virtqueue_add(struct virtqueue *_vq, struct scatterlist *sgs[],
              unsigned int total_sg,
              unsigned int out_sgs, unsigned int in_sgs,
              void *data, void *ctx, gfp_t gfp)
{
    return virtqueue_add_split(_vq, sgs, total_sg, out_sgs, in_sgs,
                               data, ctx, gfp);
}

/**
 * virtqueue_add_sgs - expose buffers to other end
 * @_vq: the struct virtqueue we're talking about.
 * @sgs: array of terminated scatterlists.
 * @out_sgs: the number of scatterlists readable by other side
 * @in_sgs: the number of scatterlists which are writable (after readable ones)
 * @data: the token identifying the buffer.
 * @gfp: how to do memory allocations (if necessary).
 *
 * Caller must ensure we don't call this with other virtqueue operations
 * at the same time (except where noted).
 *
 * Returns zero or a negative error (ie. ENOSPC, ENOMEM, EIO).
 */
int
virtqueue_add_sgs(struct virtqueue *_vq,
                  struct scatterlist *sgs[],
                  unsigned int out_sgs,
                  unsigned int in_sgs,
                  void *data,
                  gfp_t gfp)
{
    unsigned int i, total_sg = 0;

    /* Count them first. */
    for (i = 0; i < out_sgs + in_sgs; i++) {
        struct scatterlist *sg;

        for (sg = sgs[i]; sg; sg = sg_next(sg))
            total_sg++;
    }
    return virtqueue_add(_vq, sgs, total_sg, out_sgs, in_sgs,
                         data, NULL, gfp);
}
EXPORT_SYMBOL(virtqueue_add_sgs);

static inline bool more_used_split(const struct vring_virtqueue *vq)
{
    return vq->last_used_idx != vq->split.vring.used->idx;
}

static inline bool more_used(const struct vring_virtqueue *vq)
{
    return more_used_split(vq);
}

irqreturn_t vring_interrupt(int irq, void *_vq)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    if (!more_used(vq)) {
        return IRQ_NONE;
    }

    if (unlikely(vq->broken))
        return IRQ_HANDLED;

    if (vq->vq.callback)
        vq->vq.callback(&vq->vq);

    return IRQ_HANDLED;
}
EXPORT_SYMBOL(vring_interrupt);

static void virtqueue_disable_cb_split(struct virtqueue *_vq)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    if (!(vq->split.avail_flags_shadow & VRING_AVAIL_F_NO_INTERRUPT)) {
        vq->split.avail_flags_shadow |= VRING_AVAIL_F_NO_INTERRUPT;
    }
}

void virtqueue_disable_cb(struct virtqueue *_vq)
{
    virtqueue_disable_cb_split(_vq);
}
EXPORT_SYMBOL(virtqueue_disable_cb);

static unsigned virtqueue_enable_cb_prepare_split(struct virtqueue *_vq)
{
    u16 last_used_idx;
    struct vring_virtqueue *vq = to_vvq(_vq);

    /* We optimistically turn back on interrupts, then check if there was
     * more to do. */
    /* Depending on the VIRTIO_RING_F_EVENT_IDX feature, we need to
     * either clear the flags bit or point the event index at the next
     * entry. Always do both to keep code simple. */
    if (vq->split.avail_flags_shadow & VRING_AVAIL_F_NO_INTERRUPT)
        vq->split.avail_flags_shadow &= ~VRING_AVAIL_F_NO_INTERRUPT;

    last_used_idx = vq->last_used_idx;
    vring_used_event(&vq->split.vring) = last_used_idx;
    return last_used_idx;
}

unsigned virtqueue_enable_cb_prepare(struct virtqueue *_vq)
{
    return virtqueue_enable_cb_prepare_split(_vq);
}
EXPORT_SYMBOL(virtqueue_enable_cb_prepare);

static bool
virtqueue_poll_split(struct virtqueue *_vq, unsigned last_used_idx)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    return (u16)last_used_idx != vq->split.vring.used->idx;
}

bool virtqueue_poll(struct virtqueue *_vq, unsigned last_used_idx)
{
    struct vring_virtqueue *vq = to_vvq(_vq);

    if (unlikely(vq->broken))
        return false;

    return virtqueue_poll_split(_vq, last_used_idx);
}
EXPORT_SYMBOL(virtqueue_poll);

bool virtqueue_enable_cb(struct virtqueue *_vq)
{
    unsigned last_used_idx = virtqueue_enable_cb_prepare(_vq);

    return !virtqueue_poll(_vq, last_used_idx);
}
EXPORT_SYMBOL(virtqueue_enable_cb);

static void
vring_unmap_one_split(const struct vring_virtqueue *vq,
                      struct vring_desc *desc)
{
    u16 flags;

    if (!vq->use_dma_api)
        return;

    panic("use dma api!");
}

static void
detach_buf_split(struct vring_virtqueue *vq, unsigned int head,
                 void **ctx)
{
    unsigned int i, j;
    __virtio16 nextflag = VRING_DESC_F_NEXT;

    /* Clear data ptr. */
    vq->split.desc_state[head].data = NULL;

    /* Put back on free list: unmap first-level descriptors and find end */
    i = head;

    while (vq->split.vring.desc[i].flags & nextflag) {
        vring_unmap_one_split(vq, &vq->split.vring.desc[i]);
        i = vq->split.vring.desc[i].next;
        vq->vq.num_free++;
    }

    vring_unmap_one_split(vq, &vq->split.vring.desc[i]);
    vq->split.vring.desc[i].next = vq->free_head;
    vq->free_head = head;

    /* Plus final descriptor */
    vq->vq.num_free++;

    if (vq->indirect) {
        u32 len;
        struct vring_desc *indir_desc =
            vq->split.desc_state[head].indir_desc;

        /* Free the indirect table, if any, now that it's unmapped. */
        if (!indir_desc)
            return;

        len = vq->split.vring.desc[head].len;

        BUG_ON(!(vq->split.vring.desc[head].flags & VRING_DESC_F_INDIRECT));
        BUG_ON(len == 0 || len % sizeof(struct vring_desc));

        for (j = 0; j < len / sizeof(struct vring_desc); j++)
            vring_unmap_one_split(vq, &indir_desc[j]);

        kfree(indir_desc);
        vq->split.desc_state[head].indir_desc = NULL;
    } else {
        panic("not indirect!");
    }
}

static void *
virtqueue_get_buf_ctx_split(struct virtqueue *_vq, unsigned int *len,
                            void **ctx)
{
    void *ret;
    unsigned int i;
    u16 last_used;
    struct vring_virtqueue *vq = to_vvq(_vq);

    if (unlikely(vq->broken)) {
        return NULL;
    }

    if (!more_used_split(vq)) {
        return NULL;
    }

    last_used = (vq->last_used_idx & (vq->split.vring.num - 1));
    i = vq->split.vring.used->ring[last_used].id;
    *len = vq->split.vring.used->ring[last_used].len;

    if (unlikely(i >= vq->split.vring.num))
        panic("id %u out of range\n", i);

    if (unlikely(!vq->split.desc_state[i].data))
        panic("id %u is not a head!\n", i);

    /* detach_buf_split clears data, so grab it now. */
    ret = vq->split.desc_state[i].data;
    detach_buf_split(vq, i, ctx);
    vq->last_used_idx++;

    return ret;
}

void *virtqueue_get_buf_ctx(struct virtqueue *_vq, unsigned int *len,
                            void **ctx)
{
    return virtqueue_get_buf_ctx_split(_vq, len, ctx);
}
EXPORT_SYMBOL(virtqueue_get_buf_ctx);

void *virtqueue_get_buf(struct virtqueue *_vq, unsigned int *len)
{
    return virtqueue_get_buf_ctx(_vq, len, NULL);
}
EXPORT_SYMBOL(virtqueue_get_buf);

// SPDX-License-Identifier: GPL-2.0+

#include <mmio.h>
#include <slab.h>
#include <errno.h>
#include <devres.h>
#include <export.h>
#include <virtio.h>
#include <platform.h>
#include <irqreturn.h>
#include <virtio_mmio.h>
#include <virtio_config.h>
#include <virtio_ring.h>

/* The alignment to use between consumer and producer parts of vring.
 * Currently hardcoded to the page size. */
#define VIRTIO_MMIO_VRING_ALIGN     PAGE_SIZE

#define to_virtio_mmio_device(_plat_dev) \
    container_of(_plat_dev, struct virtio_mmio_device, vdev)

bool virtio_mmio_ready = false;
EXPORT_SYMBOL(virtio_mmio_ready);

struct virtio_mmio_device {
    struct virtio_device vdev;
    struct platform_device *pdev;

    void *base;
    unsigned long version;

    /* a list of queues so we can dispatch IRQs */
    struct list_head virtqueues;
};

struct virtio_mmio_vq_info {
    /* the actual virtqueue */
    struct virtqueue *vq;

    /* the list node for the virtqueues list */
    struct list_head node;
};

/* Platform driver */

static const struct of_device_id virtio_mmio_match[] = {
    { .compatible = "virtio,mmio", },
    {},
};

static u8
vm_get_status(struct virtio_device *vdev)
{
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    return readl(vm_dev->base + VIRTIO_MMIO_STATUS) & 0xff;
}

static void
vm_set_status(struct virtio_device *vdev, u8 status)
{
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    /* We should never be setting status to 0. */
    BUG_ON(status == 0);

    writel(status, vm_dev->base + VIRTIO_MMIO_STATUS);
}

static void
vm_reset(struct virtio_device *vdev)
{
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    /* 0 status means a reset. */
    writel(0, vm_dev->base + VIRTIO_MMIO_STATUS);
}

static u64
vm_get_features(struct virtio_device *vdev)
{
    u64 features;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    writel(1, vm_dev->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);
    features = readl(vm_dev->base + VIRTIO_MMIO_DEVICE_FEATURES);
    features <<= 32;

    writel(0, vm_dev->base + VIRTIO_MMIO_DEVICE_FEATURES_SEL);
    features |= readl(vm_dev->base + VIRTIO_MMIO_DEVICE_FEATURES);

    return features;
}

static int
vm_finalize_features(struct virtio_device *vdev)
{
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    /* Give virtio_ring a chance to accept features. */
    vring_transport_features(vdev);

    /* Make sure there is are no mixed devices */
    if (vm_dev->version == 2 &&
            !__virtio_test_bit(vdev, VIRTIO_F_VERSION_1)) {
        panic("New virtio-mmio devices (version 2) must provide VIRTIO_F_VERSION_1 feature!\n");
        return -EINVAL;
    }

    writel(1, vm_dev->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
    writel((u32)(vdev->features >> 32),
           vm_dev->base + VIRTIO_MMIO_DRIVER_FEATURES);

    writel(0, vm_dev->base + VIRTIO_MMIO_DRIVER_FEATURES_SEL);
    writel((u32)vdev->features,
           vm_dev->base + VIRTIO_MMIO_DRIVER_FEATURES);

    return 0;
}

static void
vm_get(struct virtio_device *vdev, unsigned offset, void *buf, unsigned len)
{
    u8 b;
    u16 w;
    u32 l;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);
    void *base = vm_dev->base + VIRTIO_MMIO_CONFIG;

    if (vm_dev->version == 1) {
        u8 *ptr = buf;
        int i;

        for (i = 0; i < len; i++)
            ptr[i] = readb(base + offset + i);
        return;
    }

    switch (len) {
    case 1:
        b = readb(base + offset);
        memcpy(buf, &b, sizeof b);
        break;
    case 2:
        w = readw(base + offset);
        memcpy(buf, &w, sizeof w);
        break;
    case 4:
        l = readl(base + offset);
        memcpy(buf, &l, sizeof l);
        break;
    case 8:
        l = readl(base + offset);
        memcpy(buf, &l, sizeof l);
        l = readl(base + offset + sizeof l);
        memcpy(buf + sizeof l, &l, sizeof l);
        break;
    default:
        BUG();
    }
}

/* the notify function used when creating a virt queue */
static bool
vm_notify(struct virtqueue *vq)
{
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vq->vdev);

    /* We write the queue's selector into the notification register to
     * signal the other end */
    writel(vq->index, vm_dev->base + VIRTIO_MMIO_QUEUE_NOTIFY);
    return true;
}

static struct virtqueue *
vm_setup_vq(struct virtio_device *vdev, unsigned index,
            void (*callback)(struct virtqueue *vq),
            const char *name, bool ctx)
{
    int err;
    u64 q_pfn;
    unsigned int num;
    struct virtqueue *vq;
    struct virtio_mmio_vq_info *info;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    if (!name)
        return NULL;

    /* Select the queue we're interested in */
    writel(index, vm_dev->base + VIRTIO_MMIO_QUEUE_SEL);

    if (vm_dev->version != 1)
        panic("version is NOT 1!");
    printk("### virtio mmio version (%u)\n", vm_dev->version);

    /* Queue shouldn't already be set up. */
    if (readl(vm_dev->base + VIRTIO_MMIO_QUEUE_PFN)) {
        err = -ENOENT;
        panic("Queue shouldn't already be set up.");
    }

    /* Allocate and fill out our active queue description */
    info = kmalloc(sizeof(*info), GFP_KERNEL);
    if (!info) {
        err = -ENOMEM;
        panic("No memory!");
    }

    num = readl(vm_dev->base + VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (num == 0) {
        err = -ENOENT;
        panic("No argument!");
    }

    /* Create the vring */
    vq = vring_create_virtqueue(index, num, VIRTIO_MMIO_VRING_ALIGN,
                                vdev, true, true, ctx, vm_notify,
                                callback, name);
    if (!vq) {
        err = -ENOMEM;
        panic("No memory!");
    }

    /* Activate the queue */
    writel(virtqueue_get_vring_size(vq), vm_dev->base + VIRTIO_MMIO_QUEUE_NUM);
    BUG_ON(vm_dev->version != 1);

    q_pfn = virtqueue_get_desc_addr(vq) >> PAGE_SHIFT;

    printk("%s: queue num(%u) new(%u) q_pfn(%lx)\n",
           __func__, num, virtqueue_get_vring_size(vq), q_pfn);

    /*
     * virtio-mmio v1 uses a 32bit QUEUE PFN. If we have something
     * that doesn't fit in 32bit, fail the setup rather than
     * pretending to be successful.
     */
    if (q_pfn >> 32) {
        panic("platform bug: legacy virtio-mmio must not be used with RAM above 0x%lxGB",
              0x1ULL << (32 + PAGE_SHIFT - 30));
    }

    writel(PAGE_SIZE, vm_dev->base + VIRTIO_MMIO_QUEUE_ALIGN);
    writel(q_pfn, vm_dev->base + VIRTIO_MMIO_QUEUE_PFN);

    vq->priv = info;
    info->vq = vq;

    list_add(&info->node, &vm_dev->virtqueues);
    return vq;
}

static void
vm_del_vq(struct virtqueue *vq)
{
    unsigned int index = vq->index;
    struct virtio_mmio_vq_info *info = vq->priv;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vq->vdev);

    list_del(&info->node);

    /* Select and deactivate the queue */
    writel(index, vm_dev->base + VIRTIO_MMIO_QUEUE_SEL);
    writel(0, vm_dev->base + VIRTIO_MMIO_QUEUE_PFN);

    // Todo:
    //vring_del_virtqueue(vq);

    kfree(info);
}

static void
vm_del_vqs(struct virtio_device *vdev)
{
    struct virtqueue *vq, *n;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);

    list_for_each_entry_safe(vq, n, &vdev->vqs, list)
        vm_del_vq(vq);
}

static irqreturn_t vm_interrupt(int irq, void *opaque)
{
    unsigned long status;
    struct virtio_mmio_vq_info *info;
    irqreturn_t ret = IRQ_NONE;
    struct virtio_mmio_device *vm_dev = opaque;

    /* Read and acknowledge interrupts */
    status = readl(vm_dev->base + VIRTIO_MMIO_INTERRUPT_STATUS);
    writel(status, vm_dev->base + VIRTIO_MMIO_INTERRUPT_ACK);

    if (unlikely(status & VIRTIO_MMIO_INT_CONFIG)) {
        panic("status & VIRTIO_MMIO_INT_CONFIG!");
        ret = IRQ_HANDLED;
    }

    if (likely(status & VIRTIO_MMIO_INT_VRING)) {
        list_for_each_entry(info, &vm_dev->virtqueues, node)
            ret |= vring_interrupt(irq, info->vq);
    }

    return ret;
}

static int
vm_find_vqs(struct virtio_device *vdev,
            unsigned nvqs, struct virtqueue *vqs[],
            vq_callback_t *callbacks[],
            const char * const names[],
            const bool *ctx,
            struct irq_affinity *desc)
{
    int i, err;
    int queue_idx = 0;
    struct virtio_mmio_device *vm_dev = to_virtio_mmio_device(vdev);
    int irq = platform_get_irq(vm_dev->pdev, 0);

    if (irq < 0)
        panic("bad irq(%d)!", irq);

    err = request_irq(irq, vm_interrupt, IRQF_SHARED,
                      dev_name(&vdev->dev), vm_dev);
    if (err)
        panic("can not request irq!");

    for (i = 0; i < nvqs; ++i) {
        if (!names[i]) {
            vqs[i] = NULL;
            continue;
        }

        vqs[i] = vm_setup_vq(vdev, queue_idx++, callbacks[i], names[i],
                             ctx ? ctx[i] : false);
        if (IS_ERR(vqs[i])) {
            vm_del_vqs(vdev);
            return PTR_ERR(vqs[i]);
        }
    }

    return 0;
}

static const struct virtio_config_ops virtio_mmio_config_ops = {
    .get        = vm_get,
    /*
    .set        = vm_set,
    .generation = vm_generation,
    */
    .reset          = vm_reset,
    .get_status     = vm_get_status,
    .set_status     = vm_set_status,
    .get_features   = vm_get_features,
    .finalize_features = vm_finalize_features,
    .find_vqs   = vm_find_vqs,
    .del_vqs    = vm_del_vqs,
    /*
    .bus_name   = vm_bus_name,
    */
};

static int
virtio_mmio_probe(struct platform_device *pdev)
{
    int rc;
    unsigned long magic;
    struct virtio_mmio_device *vm_dev;

    vm_dev = devm_kzalloc(&pdev->dev, sizeof(*vm_dev), GFP_KERNEL);
    if (!vm_dev)
        return -ENOMEM;

    vm_dev->vdev.dev.parent = &pdev->dev;
    vm_dev->vdev.config = &virtio_mmio_config_ops;
    vm_dev->pdev = pdev;
    INIT_LIST_HEAD(&vm_dev->virtqueues);

    vm_dev->base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(vm_dev->base))
        return PTR_ERR(vm_dev->base);

    /* Check magic value */
    magic = readl(vm_dev->base + VIRTIO_MMIO_MAGIC_VALUE);
    if (magic != ('v' | 'i' << 8 | 'r' << 16 | 't' << 24)) {
        panic("Wrong magic value 0x%lx!\n", magic);
        return -ENODEV;
    }

    /* Check device version */
    vm_dev->version = readl(vm_dev->base + VIRTIO_MMIO_VERSION);
    if (vm_dev->version < 1 || vm_dev->version > 2) {
        panic("Version %ld not supported!\n", vm_dev->version);
        return -ENXIO;
    }
    BUG_ON(vm_dev->version != 1);

    vm_dev->vdev.id.device = readl(vm_dev->base + VIRTIO_MMIO_DEVICE_ID);
    if (vm_dev->vdev.id.device == 0) {
        /*
         * virtio-mmio device with an ID 0 is a (dummy) placeholder
         * with no function. End probing now with no error reported.
         */
        return -ENODEV;
    }
    vm_dev->vdev.id.vendor = readl(vm_dev->base + VIRTIO_MMIO_VENDOR_ID);

    writel(PAGE_SIZE, vm_dev->base + VIRTIO_MMIO_GUEST_PAGE_SIZE);

    platform_set_drvdata(pdev, vm_dev);

    rc = register_virtio_device(&vm_dev->vdev);
    if (rc)
        put_device(&vm_dev->vdev.dev);

    printk("%s: %s ok!\n", __func__, pdev->name);
    return rc;
}

static struct platform_driver virtio_mmio_driver = {
    .probe      = virtio_mmio_probe,
    .driver     = {
        .name   = "virtio-mmio",
        .of_match_table = virtio_mmio_match,
    },
};

int
virtio_mmio_init(void)
{
    return platform_driver_register(&virtio_mmio_driver);
}
EXPORT_SYMBOL(virtio_mmio_init);

static int
init_module(void)
{
    printk("module[virtio_mmio]: init begin ...\n");
    virtio_mmio_init();
    virtio_mmio_ready = true;
    printk("module[virtio_mmio]: init end!\n");
    return 0;
}

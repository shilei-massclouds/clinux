// SPDX-License-Identifier: GPL-2.0-only

#include <list.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <devres.h>
#include <ioport.h>
#include <ioremap.h>

struct devres_node {
    struct list_head    entry;
    dr_release_t        release;
};

struct devres {
    struct devres_node      node;
    /*
     * Some archs want to perform DMA into kmalloc caches
     * and need a guaranteed alignment larger than
     * the alignment of a 64-bit integer.
     * Thus we use ARCH_KMALLOC_MINALIGN here and get exactly the same
     * buffer alignment as if it was allocated by plain kmalloc().
     */
    u8 __aligned(ARCH_KMALLOC_MINALIGN) data[];
};

struct region_devres {
    struct resource *parent;
    resource_size_t start;
    resource_size_t n;
};

enum devm_ioremap_type {
    DEVM_IOREMAP = 0,
    DEVM_IOREMAP_UC,
    DEVM_IOREMAP_WC,
};

struct resource iomem_resource = {
    .name   = "PCI mem",
    .start  = 0,
    .end    = -1,
    .flags  = IORESOURCE_MEM,
};
EXPORT_SYMBOL(iomem_resource);

static void
devm_kmalloc_release(struct device *dev, void *res)
{
    /* noop */
}

static bool
check_dr_size(size_t size, size_t *tot_size)
{
    *tot_size = sizeof(struct devres) + size;
    return true;
}

static __always_inline struct devres *
alloc_dr(dr_release_t release, size_t size, gfp_t gfp)
{
    struct devres *dr;
    size_t tot_size;

    if (!check_dr_size(size, &tot_size))
        return NULL;

    dr = kmalloc(tot_size, gfp);
    if (unlikely(!dr))
        return NULL;

    memset(dr, 0, offsetof(struct devres, data));

    INIT_LIST_HEAD(&dr->node.entry);
    dr->node.release = release;
    return dr;
}

void *
devres_alloc_node(dr_release_t release, size_t size, gfp_t gfp)
{
    struct devres *dr;

    dr = alloc_dr(release, size, gfp | __GFP_ZERO);
    if (unlikely(!dr))
        return NULL;
    return dr->data;
}
EXPORT_SYMBOL(devres_alloc_node);

void *
devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
    struct devres *dr;

    if (unlikely(!size))
        return ZERO_SIZE_PTR;

    /* use raw alloc_dr for kmalloc caller tracing */
    dr = alloc_dr(devm_kmalloc_release, size, gfp);
    if (unlikely(!dr))
        return NULL;

    /*
     * This is named devm_kzalloc_release for historical reasons
     * The initial implementation did not support kmalloc, only kzalloc
     */
    devres_add(dev, dr->data);
    return dr->data;
}
EXPORT_SYMBOL(devm_kmalloc);

char *
devm_kvasprintf(struct device *dev, gfp_t gfp, const char *fmt, va_list ap)
{
    unsigned int len;
    char *p;
    va_list aq;

    va_copy(aq, ap);
    len = vsnprintf(NULL, 0, fmt, aq);
    va_end(aq);

    p = devm_kmalloc(dev, len+1, gfp);
    if (!p)
        return NULL;

    vsnprintf(p, len+1, fmt, ap);
    return p;
}

EXPORT_SYMBOL(devm_kvasprintf);
char *
devm_kasprintf(struct device *dev, gfp_t gfp, const char *fmt, ...)
{
    va_list ap;
    char *p;

    va_start(ap, fmt);
    p = devm_kvasprintf(dev, gfp, fmt, ap);
    va_end(ap);

    return p;
}
EXPORT_SYMBOL(devm_kasprintf);

char *
devm_kstrdup(struct device *dev, const char *s, gfp_t gfp)
{
    size_t size;
    char *buf;

    if (!s)
        return NULL;

    size = strlen(s) + 1;
    buf = devm_kmalloc(dev, size, gfp);
    if (buf)
        memcpy(buf, s, size);
    return buf;
}
EXPORT_SYMBOL(devm_kstrdup);

void
__release_region(struct resource *parent,
                 resource_size_t start,
                 resource_size_t n)
{
    panic("%s not work!", __func__);
}

static void
devm_region_release(struct device *dev, void *res)
{
    struct region_devres *this = res;

    __release_region(this->parent, this->start, this->n);
}

static struct resource *bootmem_resource_free;

static struct resource *
alloc_resource(gfp_t flags)
{
    struct resource *res = NULL;

    if (bootmem_resource_free) {
        res = bootmem_resource_free;
        bootmem_resource_free = res->sibling;
    }

    if (res)
        memset(res, 0, sizeof(struct resource));
    else
        res = kzalloc(sizeof(struct resource), flags);

    return res;
}

static struct resource *
__request_resource(struct resource *root, struct resource *new)
{
    resource_size_t start = new->start;
    resource_size_t end = new->end;
    struct resource *tmp, **p;

    if (end < start)
        return root;
    if (start < root->start)
        return root;
    if (end > root->end)
        return root;
    p = &root->child;
    for (;;) {
        tmp = *p;
        if (!tmp || tmp->start > end) {
            new->sibling = tmp;
            *p = new;
            new->parent = root;
            return NULL;
        }
        p = &tmp->sibling;
        if (tmp->end < start)
            continue;
        return tmp;
    }
}

struct resource *
__request_region(struct resource *parent,
                 resource_size_t start,
                 resource_size_t n,
                 const char *name,
                 int flags)
{
    struct resource *res = alloc_resource(GFP_KERNEL);

    if (!res)
        return NULL;

    res->name = name;
    res->start = start;
    res->end = start + n - 1;

    for (;;) {
        struct resource *conflict;

        res->flags = resource_type(parent) | resource_ext_type(parent);
        res->flags |= IORESOURCE_BUSY | flags;
        res->desc = parent->desc;

        conflict = __request_resource(parent, res);
        if (!conflict)
            break;

        panic("%s: find conflict!", __func__);
    }

    return res;
}
EXPORT_SYMBOL(__request_region);

struct resource *
__devm_request_region(struct device *dev,
                      struct resource *parent,
                      resource_size_t start,
                      resource_size_t n,
                      const char *name)
{
    struct region_devres *dr = NULL;
    struct resource *res;

    dr = devres_alloc(devm_region_release,
                      sizeof(struct region_devres),
                      GFP_KERNEL);
    if (!dr)
        return NULL;

    dr->parent = parent;
    dr->start = start;
    dr->n = n;

    res = __request_region(parent, start, n, name, 0);
    if (res)
        devres_add(dev, dr);
    else
        devres_free(dr);

    return res;
}
EXPORT_SYMBOL(__devm_request_region);

static inline void
iounmap(void *addr)
{
    panic("not this function!");
}

static inline void *
ioremap_uc(phys_addr_t addr, size_t size)
{
    panic("not this function!");
}

static inline void *
ioremap_wc(phys_addr_t addr, size_t size)
{
    panic("not this function!");
}

void
devm_ioremap_release(struct device *dev, void *res)
{
    iounmap(*(void **)res);
}

static void *
__devm_ioremap(struct device *dev, resource_size_t offset,
               resource_size_t size, enum devm_ioremap_type type)
{
    void **ptr, *addr = NULL;

    ptr = devres_alloc(devm_ioremap_release, sizeof(*ptr), GFP_KERNEL);
    if (!ptr)
        return NULL;

    switch (type) {
    case DEVM_IOREMAP:
        addr = ioremap(offset, size);
        break;
    case DEVM_IOREMAP_UC:
        addr = ioremap_uc(offset, size);
        break;
    case DEVM_IOREMAP_WC:
        addr = ioremap_wc(offset, size);
        break;
    }

    if (addr) {
        *ptr = addr;
        devres_add(dev, ptr);
    } else
        devres_free(ptr);

    return addr;
}

static void *
__devm_ioremap_resource(struct device *dev,
                        const struct resource *res,
                        enum devm_ioremap_type type)
{
    resource_size_t size;
    void *dest_ptr;
    char *pretty_name;

    BUG_ON(!dev);

    if (!res || resource_type(res) != IORESOURCE_MEM) {
        panic("invalid resource");
        return IOMEM_ERR_PTR(-EINVAL);
    }

    size = resource_size(res);

    if (res->name)
        pretty_name = devm_kasprintf(dev, GFP_KERNEL, "%s %s",
                                     dev_name(dev), res->name);
    else
        pretty_name = devm_kstrdup(dev, dev_name(dev), GFP_KERNEL);
    if (!pretty_name)
        return IOMEM_ERR_PTR(-ENOMEM);

    if (!devm_request_mem_region(dev, res->start, size, pretty_name)) {
        panic("can't request region for resource %p\n", res);
        return IOMEM_ERR_PTR(-EBUSY);
    }

    dest_ptr = __devm_ioremap(dev, res->start, size, type);
    if (!dest_ptr) {
        panic("ioremap failed for resource %p\n", res);
        devm_release_mem_region(dev, res->start, size);
        dest_ptr = IOMEM_ERR_PTR(-ENOMEM);
    }

    return dest_ptr;
}

void *
devm_ioremap_resource(struct device *dev, const struct resource *res)
{
    return __devm_ioremap_resource(dev, res, DEVM_IOREMAP);
}
EXPORT_SYMBOL(devm_ioremap_resource);

void
__devm_release_region(struct device *dev, struct resource *parent,
                      resource_size_t start, resource_size_t n)
{
    /* Todo */
}
EXPORT_SYMBOL(__devm_release_region);

static void
add_dr(struct device *dev, struct devres_node *node)
{
    BUG_ON(!list_empty(&node->entry));
    list_add_tail(&node->entry, &dev->devres_head);
}

void
devres_add(struct device *dev, void *res)
{
    struct devres *dr = container_of(res, struct devres, data);
    add_dr(dev, &dr->node);
}
EXPORT_SYMBOL(devres_add);

void
devres_free(void *res)
{
    if (res) {
        struct devres *dr = container_of(res, struct devres, data);

        BUG_ON(!list_empty(&dr->node.entry));
        kfree(dr);
    }
}
EXPORT_SYMBOL(devres_free);

int
init_module(void)
{
    printk("module[devres]: init begin ...\n");
    printk("module[devres]: init end!\n");
    return 0;
}

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_IOPORT_H
#define _LINUX_IOPORT_H

#include <types.h>

#define IOMEM_ERR_PTR(err) (__force void *)ERR_PTR(err)

#define IORESOURCE_TYPE_BITS    0x00001f00  /* Resource type */
#define IORESOURCE_IO           0x00000100  /* PCI/ISA I/O ports */
#define IORESOURCE_MEM          0x00000200

#define IORESOURCE_EXT_TYPE_BITS 0x01000000 /* Resource extended types */

#define IORESOURCE_BUSY     0x80000000  /* Driver has marked this resource busy */

#define devm_request_mem_region(dev,start,n,name) \
    __devm_request_region(dev, &iomem_resource, (start), (n), (name))

#define devm_release_mem_region(dev, start, n) \
    __devm_release_region(dev, &iomem_resource, (start), (n))

#define request_mem_region(start,n,name) \
    __request_region(&iomem_resource, (start), (n), (name), 0)

extern struct resource iomem_resource;

extern struct resource *
__request_region(struct resource *,
                 resource_size_t start, resource_size_t n,
                 const char *name, int flags);

struct device;

/*
 * Resources are tree-like, allowing
 * nesting etc..
 */
struct resource {
    resource_size_t start;
    resource_size_t end;
    const char *name;
    unsigned long flags;
    unsigned long desc;
    struct resource *parent, *sibling, *child;
};

static inline unsigned long
resource_type(const struct resource *res)
{
    return res->flags & IORESOURCE_TYPE_BITS;
}

static inline resource_size_t
resource_size(const struct resource *res)
{
    return res->end - res->start + 1;
}

static inline unsigned long
resource_ext_type(const struct resource *res)
{
    return res->flags & IORESOURCE_EXT_TYPE_BITS;
}

struct resource *
__devm_request_region(struct device *dev,
                      struct resource *parent,
                      resource_size_t start,
                      resource_size_t n,
                      const char *name);

void
__devm_release_region(struct device *dev, struct resource *parent,
                      resource_size_t start, resource_size_t n);

#endif  /* _LINUX_IOPORT_H */

// SPDX-License-Identifier: GPL-2.0

#include <fdt.h>
#include <errno.h>
#include <export.h>
#include <ioremap.h>
#include <of_address.h>

/* Max address size we deal with */
#define OF_MAX_ADDR_CELLS   4
#define OF_CHECK_ADDR_COUNT(na) ((na) > 0 && (na) <= OF_MAX_ADDR_CELLS)
#define OF_CHECK_COUNTS(na, ns) (OF_CHECK_ADDR_COUNT(na) && (ns) > 0)

struct of_bus {
    const char *name;
    const char *addresses;
    int (*match)(struct device_node *parent);
    void (*count_cells)(struct device_node *child, int *addrc, int *sizec);
    u64 (*map)(u32 *addr, const u32 *range, int na, int ns, int pna);
    int (*translate)(u32 *addr, u64 offset, int na);
    unsigned int (*get_flags)(const u32 *addr);
};

static void
of_bus_default_count_cells(struct device_node *dev,
                           int *addrc,
                           int *sizec)
{
    if (addrc)
        *addrc = of_n_addr_cells(dev);
    if (sizec)
        *sizec = of_n_size_cells(dev);
}

static unsigned int
of_bus_default_get_flags(const u32 *addr)
{
    return IORESOURCE_MEM;
}

static u64
of_bus_default_map(u32 *addr, const u32 *range,
                   int na, int ns, int pna)
{
    u64 cp, s, da;

    cp = of_read_number(range, na);
    s  = of_read_number(range + na + pna, ns);
    da = of_read_number(addr, na);

    pr_debug("default map, cp=%lx, s=%lx, da=%lx\n",
             (unsigned long long)cp, (unsigned long long)s,
             (unsigned long long)da);

    if (da < cp || da >= (cp + s))
        return OF_BAD_ADDR;
    return da - cp;
}

static int
of_bus_default_translate(u32 *addr, u64 offset, int na)
{
    u64 a = of_read_number(addr, na);
    memset(addr, 0, na * 4);
    a += offset;
    if (na > 1)
        addr[na - 2] = cpu_to_be32(a >> 32);
    addr[na - 1] = cpu_to_be32(a & 0xffffffffu);

    return 0;
}

static int
of_empty_ranges_quirk(struct device_node *np)
{
    return false;
}

static int
of_translate_one(struct device_node *parent, struct of_bus *bus,
                 struct of_bus *pbus, u32 *addr,
                 int na, int ns, int pna, const char *rprop)
{
    const u32 *ranges;
    unsigned int rlen;
    int rone;
    u64 offset = OF_BAD_ADDR;

    ranges = of_get_property(parent, rprop, &rlen);
    if (ranges == NULL && !of_empty_ranges_quirk(parent) &&
        strcmp(rprop, "dma-ranges")) {
        panic("no ranges; cannot translate\n");
        return 1;
    }
    if (ranges == NULL || rlen == 0) {
        offset = of_read_number(addr, na);
        memset(addr, 0, pna * 4);
        pr_debug("empty ranges; 1:1 translation\n");
        goto finish;
    }

    pr_debug("walking ranges...\n");

    /* Now walk through the ranges */
    rlen /= 4;
    rone = na + pna + ns;
    for (; rlen >= rone; rlen -= rone, ranges += rone) {
        offset = bus->map(addr, ranges, na, ns, pna);
        if (offset != OF_BAD_ADDR)
            break;
    }
    if (offset == OF_BAD_ADDR) {
        pr_debug("not found !\n");
        return 1;
    }
    memcpy(addr, ranges + na, 4 * pna);

 finish:
    pr_debug("with offset: %lx\n", (unsigned long long)offset);

    /* Translate it into parent bus space */
    return pbus->translate(addr, offset, pna);
}

static struct of_bus of_busses[] = {
    /* Default */
    {
        .name = "default",
        .addresses = "reg",
        .match = NULL,
        .count_cells = of_bus_default_count_cells,
        .map = of_bus_default_map,
        .translate = of_bus_default_translate,
        .get_flags = of_bus_default_get_flags,
    },
};

static struct of_bus *
of_match_bus(struct device_node *np)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(of_busses); i++)
        if (!of_busses[i].match || of_busses[i].match(np))
            return &of_busses[i];
    BUG();
    return NULL;
}

static u64
__of_translate_address(struct device_node *dev,
                       struct device_node *(*get_parent)(const struct device_node *),
                       const u32 *in_addr,
                       const char *rprop,
                       struct device_node **host)
{
    int na, ns;
    int pna, pns;
    struct of_bus *bus;
    struct of_bus *pbus;
    u32 addr[OF_MAX_ADDR_CELLS];
    struct device_node *parent = NULL;
    u64 result = OF_BAD_ADDR;

    /* Increase refcount at current level */
    of_node_get(dev);

    *host = NULL;
    /* Get parent & match bus type */
    parent = get_parent(dev);
    if (parent == NULL)
        goto bail;
    bus = of_match_bus(parent);

    /* Count address cells & copy address locally */
    bus->count_cells(dev, &na, &ns);
    if (!OF_CHECK_COUNTS(na, ns)) {
        pr_debug("Bad cell count for %lxOF\n", dev);
        goto bail;
    }
    memcpy(addr, in_addr, na * 4);

    /* Translate */
    for (;;) {
        struct logic_pio_hwaddr *iorange;

        /* Switch to parent bus */
        of_node_put(dev);
        dev = parent;
        parent = get_parent(dev);

        /* If root, we have finished */
        if (parent == NULL) {
            result = of_read_number(addr, na);
            pr_debug("reached root node (%lx)\n", result);
            break;
        }

        /* Get new parent bus and counts */
        pbus = of_match_bus(parent);
        pbus->count_cells(dev, &pna, &pns);
        if (!OF_CHECK_COUNTS(pna, pns)) {
            panic("Bad cell count for %p", dev);
            break;
        }

        pr_debug("parent bus is %s (na=%d, ns=%d) on %p\n",
                 pbus->name, pna, pns, parent);

        /* Apply bus translation */
        if (of_translate_one(dev, bus, pbus, addr, na, ns, pna, rprop))
            break;

        /* Complete the move up one level */
        na = pna;
        ns = pns;
        bus = pbus;

        pr_debug("one level translation: (%lx, %x)\n", addr, na);
    }

 bail:
    of_node_put(parent);
    of_node_put(dev);

    return result;
}

u64
of_translate_address(struct device_node *dev, const u32 *in_addr)
{
    struct device_node *host;
    u64 ret;

    ret = __of_translate_address(dev, of_get_parent, in_addr, "ranges", &host);
    if (host) {
        of_node_put(host);
        return OF_BAD_ADDR;
    }

    return ret;
}
EXPORT_SYMBOL(of_translate_address);

static int
__of_address_to_resource(struct device_node *dev,
                         const u32 *addrp,
                         u64 size,
                         unsigned int flags,
                         const char *name,
                         struct resource *r)
{
    u64 taddr;

    if (flags & IORESOURCE_MEM)
        taddr = of_translate_address(dev, addrp);
    else if (flags & IORESOURCE_IO)
        panic("not support resource io!");
    else
        return -EINVAL;

    if (taddr == OF_BAD_ADDR)
        return -EINVAL;
    memset(r, 0, sizeof(struct resource));

    r->start = taddr;
    r->end = taddr + size - 1;
    r->flags = flags;
    r->name = name ? name : dev->full_name;
    return 0;
}

int
of_address_to_resource(struct device_node *dev,
                       int index,
                       struct resource *r)
{
    u64 size;
    const u32 *addrp;
    unsigned int flags;
    const char *name = NULL;

    addrp = of_get_address(dev, index, &size, &flags);
    if (addrp == NULL)
        return -EINVAL;

    return __of_address_to_resource(dev, addrp, size, flags, name, r);
}
EXPORT_SYMBOL(of_address_to_resource);

/**
 * of_iomap - Maps the memory mapped IO for a given device_node
 * @np:     the device whose io range will be mapped
 * @index:  index of the io range
 *
 * Returns a pointer to the mapped memory
 */
void *of_iomap(struct device_node *np, int index)
{
    struct resource res;

    if (of_address_to_resource(np, index, &res))
        return NULL;

    return ioremap(res.start, resource_size(&res));
}
EXPORT_SYMBOL(of_iomap);

const u32 *
of_get_address(struct device_node *dev,
               int index,
               u64 *size,
               unsigned int *flags)
{
    int i;
    const u32 *prop;
    unsigned int psize;
    struct device_node *parent;
    struct of_bus *bus;
    int onesize;
    int na, ns;

    parent = of_get_parent(dev);
    if (parent == NULL)
        return NULL;

    bus = of_match_bus(parent);
    bus->count_cells(dev, &na, &ns);
    of_node_put(parent);

    if (!OF_CHECK_ADDR_COUNT(na))
        return NULL;

    /* Get "reg" or "assigned-addresses" property */
    prop = of_get_property(dev, bus->addresses, &psize);
    if (prop == NULL)
        return NULL;
    psize /= 4;

    onesize = na + ns;
    for (i = 0; psize >= onesize; psize -= onesize, prop += onesize, i++) {
        if (i == index) {
            if (size)
                *size = of_read_number(prop + na, ns);
            if (flags)
                *flags = bus->get_flags(prop);

            return prop;
        }
    }
    return NULL;
}
EXPORT_SYMBOL(of_get_address);

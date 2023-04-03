// SPDX-License-Identifier: GPL-2.0+

#include <of.h>
#include <bug.h>
#include <irq.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <irqchip.h>
#include <irqdesc.h>
#include <irqdomain.h>

extern int nr_irqs;
extern struct irq_chip no_irq_chip;

static LIST_HEAD(irq_domain_list);

const struct fwnode_operations irqchip_fwnode_ops;
EXPORT_SYMBOL(irqchip_fwnode_ops);

static void irq_domain_set_mapping(struct irq_domain *domain,
                                   irq_hw_number_t hwirq,
                                   struct irq_data *irq_data)
{
    if (hwirq < domain->revmap_size)
        domain->linear_revmap[hwirq] = irq_data->irq;
    else
        panic("hwirq too large!");
}

struct irq_domain *
irq_find_matching_fwspec(struct irq_fwspec *fwspec,
                         enum irq_domain_bus_token bus_token)
{
    struct irq_domain *h;
    struct fwnode_handle *fwnode = fwspec->fwnode;

    list_for_each_entry(h, &irq_domain_list, link) {
        printk("%s: name(%s) (%p, %p) (%u)\n",
               __func__, h->name, fwnode, h->fwnode, bus_token);
        if (fwnode && (h->fwnode == fwnode) &&
            ((bus_token == DOMAIN_BUS_ANY) ||
             (h->bus_token == bus_token)))
            return h;
    }

    return NULL;
}
EXPORT_SYMBOL(irq_find_matching_fwspec);

int irq_domain_associate(struct irq_domain *domain,
                         unsigned int virq, irq_hw_number_t hwirq)
{
    int ret;
    struct irq_data *irq_data = irq_get_irq_data(virq);

    BUG_ON(!irq_data);
    BUG_ON(irq_data->domain);

    irq_data->hwirq = hwirq;
    irq_data->domain = domain;

    if (domain->ops->map) {
        ret = domain->ops->map(domain, virq, hwirq);
        if (ret != 0)
            panic("map failed!");

        /* If not already assigned, give the domain the chip's name */
        if (!domain->name && irq_data->chip)
            domain->name = irq_data->chip->name;
    }

    irq_domain_set_mapping(domain, hwirq, irq_data);
    return 0;
}

unsigned int irq_create_mapping(struct irq_domain *domain,
                                irq_hw_number_t hwirq)
{
    int virq;
    struct device_node *of_node;

    BUG_ON(!domain);

    of_node = irq_domain_get_of_node(domain);

    /* Allocate a virtual interrupt number */
    virq = irq_domain_alloc_descs(-1, 1, hwirq, NULL);
    if (virq <= 0)
        panic("-> virq allocation failed\n");

    if (irq_domain_associate(domain, virq, hwirq))
        panic("can not assocate!");

    return virq;
}

static int
irq_domain_translate(struct irq_domain *d,
                     struct irq_fwspec *fwspec,
                     irq_hw_number_t *hwirq, unsigned int *type)
{
    printk("%s: 1 ops(%p) translate\n", __func__, d->ops);
    if (d->ops->translate)
        return d->ops->translate(d, fwspec, hwirq, type);
    if (d->ops->xlate) {
        printk("%s: 2 xlate (%x)\n", __func__, fwspec->param[0]);
        return d->ops->xlate(d, to_of_node(fwspec->fwnode),
                             fwspec->param, fwspec->param_count,
                             hwirq, type);
    }

    printk("%s: 3 no\n", __func__);
    /* If domain has no translation, then we assume interrupt line */
    *hwirq = fwspec->param[0];
    return 0;
}

unsigned int irq_create_fwspec_mapping(struct irq_fwspec *fwspec)
{
    int virq;
    irq_hw_number_t hwirq;
    struct irq_domain *domain;
    unsigned int type = IRQ_TYPE_NONE;

    if (fwspec->fwnode) {
        domain = irq_find_matching_fwspec(fwspec, DOMAIN_BUS_ANY);
    } else {
        panic("fwnode is NULL!");
    }

    if (!domain)
        panic("no irq domain found!");

    if (irq_domain_translate(domain, fwspec, &hwirq, &type))
        return 0;

    if (irq_domain_is_hierarchy(domain)) {
        virq = irq_domain_alloc_irqs(domain, 1, NUMA_NO_NODE, fwspec);
        if (virq <= 0)
            return 0;
    } else {
        virq = irq_create_mapping(domain, hwirq);
        if (!virq)
            return 0;
    }

    return virq;
}

static void
of_phandle_args_to_fwspec(struct device_node *np,
                          const u32 *args,
                          unsigned int count,
                          struct irq_fwspec *fwspec)
{
    int i;

    fwspec->fwnode = np ? &np->fwnode : NULL;
    fwspec->param_count = count;

    for (i = 0; i < count; i++)
        fwspec->param[i] = args[i];
}

unsigned int irq_create_of_mapping(struct of_phandle_args *irq_data)
{
    struct irq_fwspec fwspec;

    of_phandle_args_to_fwspec(irq_data->np, irq_data->args,
                              irq_data->args_count, &fwspec);

    return irq_create_fwspec_mapping(&fwspec);
}
EXPORT_SYMBOL(irq_create_of_mapping);

static void irq_domain_check_hierarchy(struct irq_domain *domain)
{
    /* Hierarchy irq_domains must implement callback alloc() */
    if (domain->ops->alloc)
        domain->flags |= IRQ_DOMAIN_FLAG_HIERARCHY;
}

struct irq_domain *
__irq_domain_add(struct fwnode_handle *fwnode, int size,
                 irq_hw_number_t hwirq_max, int direct_max,
                 const struct irq_domain_ops *ops,
                 void *host_data)
{
    struct irq_domain *domain;

    domain = kzalloc(sizeof(*domain) + (sizeof(unsigned int) * size),
                     GFP_KERNEL);
    if (!domain)
        return NULL;

    if (is_fwnode_irqchip(fwnode)) {
        panic("fwnode is NOT irqchip!");
    } else if (is_of_node(fwnode)) {
        char *name;

        /*
         * fwnode paths contain '/', which debugfs is legitimately
         * unhappy about. Replace them with ':', which does
         * the trick and is not as offensive as '\'...
         */
        name = kasprintf(GFP_KERNEL, "%pfw", fwnode);
        if (!name) {
            kfree(domain);
            return NULL;
        }

        strreplace(name, '/', ':');

        domain->name = name;
        domain->fwnode = fwnode;
    } else {
        panic("check fwnode error!");
    }

    domain->ops = ops;
    domain->host_data = host_data;
    domain->revmap_size = size;
    irq_domain_check_hierarchy(domain);

    printk("%s: ops(%p)\n", __func__, ops);
    list_add(&domain->link, &irq_domain_list);
    return domain;
}
EXPORT_SYMBOL(__irq_domain_add);

int
irq_domain_alloc_irqs_hierarchy(struct irq_domain *domain,
                                unsigned int irq_base,
                                unsigned int nr_irqs, void *arg)
{
    if (!domain->ops->alloc) {
        panic("domain->ops->alloc() is NULL");
        return -ENOSYS;
    }

    return domain->ops->alloc(domain, irq_base, nr_irqs, arg);
}

int irq_domain_alloc_descs(int virq, unsigned int cnt,
                           irq_hw_number_t hwirq,
                           const struct irq_affinity_desc *affinity)
{
    unsigned int hint;

    if (virq >= 0) {
        virq = __irq_alloc_descs(virq, virq, cnt, affinity);
    } else {
        hint = hwirq % nr_irqs;
        if (hint == 0)
            hint++;
        printk("%s: 1 virq(%d)\n", __func__, virq);
        virq = __irq_alloc_descs(-1, hint, cnt, affinity);
        printk("%s: 2 virq(%d)\n", __func__, virq);
        if (virq <= 0 && hint > 1) {
            virq = __irq_alloc_descs(-1, 1, cnt, affinity);
        }
    }

    return virq;
}

static struct irq_data *
irq_domain_insert_irq_data(struct irq_domain *domain, struct irq_data *child)
{
    struct irq_data *irq_data;

    irq_data = kzalloc(sizeof(*irq_data), GFP_KERNEL);
    if (irq_data) {
        child->parent_data = irq_data;
        irq_data->irq = child->irq;
        irq_data->domain = domain;
    }

    return irq_data;
}

static int
irq_domain_alloc_irq_data(struct irq_domain *domain,
                          unsigned int virq, unsigned int nr_irqs)
{
    int i;
    struct irq_data *irq_data;
    struct irq_domain *parent;

    /* The outermost irq_data is embedded in struct irq_desc */
    for (i = 0; i < nr_irqs; i++) {
        irq_data = irq_get_irq_data(virq + i);
        irq_data->domain = domain;

        for (parent = domain->parent; parent; parent = parent->parent) {
            irq_data = irq_domain_insert_irq_data(parent, irq_data);
            if (!irq_data) {
                panic("no memory!");
            }
        }
    }

    return 0;
}

static void irq_domain_insert_irq(int virq)
{
    struct irq_data *data;

    for (data = irq_get_irq_data(virq); data; data = data->parent_data) {
        struct irq_domain *domain = data->domain;

        irq_domain_set_mapping(domain, data->hwirq, data);

        /* If not already assigned, give the domain the chip's name */
        if (!domain->name && data->chip)
            domain->name = data->chip->name;
    }
}

int
__irq_domain_alloc_irqs(struct irq_domain *domain, int irq_base,
                        unsigned int nr_irqs, int node, void *arg,
                        bool realloc,
                        const struct irq_affinity_desc *affinity)
{
    int i, ret, virq;

    BUG_ON(realloc);

    virq = irq_domain_alloc_descs(irq_base, nr_irqs, 0, affinity);
    if (virq < 0) {
        panic("cannot allocate IRQ(base %d, count %d)", irq_base, nr_irqs);
        return virq;
    }

    if (irq_domain_alloc_irq_data(domain, virq, nr_irqs))
        panic("can not alloc irq data!");

    ret = irq_domain_alloc_irqs_hierarchy(domain, virq, nr_irqs, arg);
    if (ret < 0)
        panic("bad alloc!");

    for (i = 0; i < nr_irqs; i++)
        irq_domain_insert_irq(virq + i);

    return virq;
}

int irq_domain_translate_onecell(struct irq_domain *d,
                                 struct irq_fwspec *fwspec,
                                 unsigned long *out_hwirq,
                                 unsigned int *out_type)
{
    BUG_ON(fwspec->param_count < 1);
    *out_hwirq = fwspec->param[0];
    *out_type = IRQ_TYPE_NONE;
    return 0;
}
EXPORT_SYMBOL(irq_domain_translate_onecell);

int irq_domain_xlate_onecell(struct irq_domain *d,
                             struct device_node *ctrlr,
                             const u32 *intspec,
                             unsigned int intsize,
                             unsigned long *out_hwirq,
                             unsigned int *out_type)
{
    BUG_ON(intsize < 1);
    *out_hwirq = intspec[0];
    *out_type = IRQ_TYPE_NONE;
    return 0;
}
EXPORT_SYMBOL(irq_domain_xlate_onecell);

struct irq_data *
irq_domain_get_irq_data(struct irq_domain *domain, unsigned int virq)
{
    struct irq_data *irq_data;

    for (irq_data = irq_get_irq_data(virq); irq_data;
         irq_data = irq_data->parent_data)
        if (irq_data->domain == domain)
            return irq_data;

    return NULL;
}
EXPORT_SYMBOL(irq_domain_get_irq_data);

int
irq_domain_set_hwirq_and_chip(struct irq_domain *domain,
                              unsigned int virq,
                              irq_hw_number_t hwirq,
                              struct irq_chip *chip,
                              void *chip_data)
{
    struct irq_data *irq_data = irq_domain_get_irq_data(domain, virq);

    if (!irq_data)
        panic("no irq data!");

    irq_data->hwirq = hwirq;
    irq_data->chip = chip ? chip : &no_irq_chip;
    irq_data->chip_data = chip_data;

    return 0;
}
EXPORT_SYMBOL(irq_domain_set_hwirq_and_chip);

void irq_domain_set_info(struct irq_domain *domain, unsigned int virq,
                         irq_hw_number_t hwirq, struct irq_chip *chip,
                         void *chip_data, irq_flow_handler_t handler,
                         void *handler_data, const char *handler_name)
{
    irq_domain_set_hwirq_and_chip(domain, virq, hwirq, chip, chip_data);
    __irq_set_handler(virq, handler, 0, handler_name);
    irq_set_chip_data(virq, chip_data);
}
EXPORT_SYMBOL(irq_domain_set_info);

/**
 * irq_find_mapping() - Find a linux irq from a hw irq number.
 * @domain: domain owning this hardware interrupt
 * @hwirq: hardware irq number in that domain space
 */
unsigned int
irq_find_mapping(struct irq_domain *domain, irq_hw_number_t hwirq)
{
    struct irq_data *data;

    BUG_ON(domain == NULL);

    pr_debug("%s: %lu, %u\n",
             __func__, hwirq, domain->revmap_size);

    /* Check if the hwirq is in the linear revmap. */
    if (hwirq < domain->revmap_size)
        return domain->linear_revmap[hwirq];

    panic("%s: !", __func__);
}
EXPORT_SYMBOL(irq_find_mapping);

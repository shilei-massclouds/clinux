// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <irq.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <of_irq.h>
#include <irqdomain.h>

struct of_intc_desc {
    struct list_head    list;
    of_irq_init_cb_t    irq_init_cb;
    struct device_node  *dev;
    struct device_node  *interrupt_parent;
};

struct device_node *of_irq_find_parent(struct device_node *child)
{
    phandle parent;
    struct device_node *p;

    if (!of_node_get(child))
        return NULL;

    do {
        if (of_property_read_u32(child, "interrupt-parent", &parent))
            p = of_get_parent(child);
        else
            p = of_find_node_by_phandle(parent);
        of_node_put(child);
        child = p;
    } while (p && of_get_property(p, "#interrupt-cells", NULL) == NULL);

    return p;
}
EXPORT_SYMBOL(of_irq_find_parent);

static void *
of_find_property_value_of_size(const struct device_node *np,
                               const char *propname,
                               u32 min,
                               u32 max,
                               size_t *len)
{
    struct property *prop = of_find_property(np, propname, NULL);

    if (!prop)
        return ERR_PTR(-EINVAL);
    if (!prop->value)
        return ERR_PTR(-ENODATA);
    if (prop->length < min)
        return ERR_PTR(-EOVERFLOW);
    if (max && prop->length > max)
        return ERR_PTR(-EOVERFLOW);

    if (len)
        *len = prop->length;

    return prop->value;
}

int of_irq_parse_raw(const u32 *addr, struct of_phandle_args *out_irq)
{
    int i;
    u32 addrsize;
    const u32 *tmp;
    u32 initial_match_array[MAX_PHANDLE_ARGS];
    struct device_node *ipar, *tnode, *old = NULL;
    u32 intsize = 1;

    ipar = of_node_get(out_irq->np);

    /* First get the #interrupt-cells property of the current cursor
     * that tells us how to interpret the passed-in intspec. If there
     * is none, we are nice and just walk up the tree
     */
    do {
        if (!of_property_read_u32(ipar, "#interrupt-cells", &intsize))
            break;
        tnode = ipar;
        ipar = of_irq_find_parent(ipar);
        of_node_put(tnode);
    } while (ipar);

    if (ipar == NULL)
        panic(" -> no parent found !\n");

    if (out_irq->args_count != intsize)
        panic("bad intsize!");

    /* Look for this #address-cells. We have to implement the old linux
     * trick of looking for the parent here as some device-trees rely on it
     */
    old = of_node_get(ipar);
    do {
        tmp = of_get_property(old, "#address-cells", NULL);
        tnode = of_get_parent(old);
        of_node_put(old);
        old = tnode;
    } while (old && tmp == NULL);
    of_node_put(old);
    old = NULL;
    addrsize = (tmp == NULL) ? 2 : be32_to_cpu(*tmp);

    /* Range check so that the temporary buffer doesn't overflow */
    BUG_ON(addrsize + intsize > MAX_PHANDLE_ARGS);

    /* Precalculate the match array - this simplifies match loop */
    for (i = 0; i < addrsize; i++)
        initial_match_array[i] = addr ? addr[i] : 0;
    for (i = 0; i < intsize; i++)
        initial_match_array[addrsize + i] = cpu_to_be32(out_irq->args[i]);

    while (ipar != NULL) {
        /* Now check if cursor is an interrupt-controller and if it is
         * then we are done
         */
        if (of_property_read_bool(ipar, "interrupt-controller")) {
            printk(" -> got it!\n");
            return 0;
        }
        panic("%s: !", __func__);
    }
    panic("%s: addrsize(%u) intsize(%u)!", __func__, addrsize, intsize);
}

int
of_irq_parse_one(struct device_node *device,
                 int index,
                 struct of_phandle_args *out_irq)
{
    int i, res;
    u32 intsize;
    const u32 *addr;
    struct device_node *p;

    /* Get the reg property (if any) */
    addr = of_get_property(device, "reg", NULL);

    printk("%s: 1\n", __func__);

    /* Try the new-style interrupts-extended first */
    res = of_parse_phandle_with_args(device,
                                     "interrupts-extended",
                                     "#interrupt-cells",
                                     index, out_irq);
    if (!res)
        return of_irq_parse_raw(addr, out_irq);

    printk("%s: 2\n", __func__);
    /* Look for the interrupt parent. */
    p = of_irq_find_parent(device);
    if (p == NULL)
        return -EINVAL;

    /* Get size of interrupt specifier */
    if (of_property_read_u32(p, "#interrupt-cells", &intsize))
        return -EINVAL;

    /* Copy intspec into irq structure */
    out_irq->np = p;
    out_irq->args_count = intsize;
    for (i = 0; i < intsize; i++) {
        res = of_property_read_u32_index(device, "interrupts",
                                         (index * intsize) + i,
                                         out_irq->args + i);
        if (res)
            return res;
    }

    res = of_irq_parse_raw(addr, out_irq);
    return res;
}
EXPORT_SYMBOL(of_irq_parse_one);

int of_irq_get(struct device_node *dev, int index)
{
    int rc;
    struct of_phandle_args oirq;

    rc = of_irq_parse_one(dev, index, &oirq);
    if (rc)
        return rc;

    return irq_create_of_mapping(&oirq);
}
EXPORT_SYMBOL(of_irq_get);

void of_irq_init(const struct of_device_id *matches)
{
    struct device_node *np;
    struct of_intc_desc *desc;
    struct list_head intc_desc_list;
    const struct of_device_id *match;

    INIT_LIST_HEAD(&intc_desc_list);

    for_each_matching_node_and_match(np, matches, &match) {
        if (!of_property_read_bool(np, "interrupt-controller") ||
            !of_device_is_available(np))
            continue;

        BUG_ON(!match->data);

        /*
         * Here, we allocate and populate an of_intc_desc with the node
         * pointer, interrupt-parent device_node etc.
         */
        desc = kzalloc(sizeof(*desc), GFP_KERNEL);
        if (!desc)
            panic("out of memory!");

        desc->irq_init_cb = match->data;
        desc->dev = of_node_get(np);
        desc->interrupt_parent = of_irq_find_parent(np);
        if (desc->interrupt_parent == np)
            desc->interrupt_parent = NULL;
        list_add_tail(&desc->list, &intc_desc_list);
    }

    while (!list_empty(&intc_desc_list)) {
        struct of_intc_desc *temp_desc;
        list_for_each_entry_safe(desc, temp_desc, &intc_desc_list, list) {
            int ret;

            list_del(&desc->list);

            of_node_set_flag(desc->dev, OF_POPULATED);

            ret = desc->irq_init_cb(desc->dev, desc->interrupt_parent);
            if (ret)
                panic("init irq err!");
        }
    }
}
EXPORT_SYMBOL(of_irq_init);

/**
 * of_irq_count - Count the number of IRQs a node uses
 * @dev: pointer to device tree node
 */
int of_irq_count(struct device_node *dev)
{
    struct of_phandle_args irq;
    int nr = 0;

    while (of_irq_parse_one(dev, nr, &irq) == 0)
        nr++;

    return nr;
}
EXPORT_SYMBOL(of_irq_count);

unsigned int irq_of_parse_and_map(struct device_node *dev, int index)
{
    struct of_phandle_args oirq;

    if (of_irq_parse_one(dev, index, &oirq))
        return 0;

    printk("%s: index(%d) before irq_create_of_mapping...\n",
           __func__, index);
    return irq_create_of_mapping(&oirq);
}
EXPORT_SYMBOL(irq_of_parse_and_map);

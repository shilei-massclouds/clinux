// SPDX-License-Identifier: GPL-2.0-only

#include <csr.h>
#include <irq.h>
#include <mmio.h>
#include <slab.h>
#include <export.h>
#include <of_irq.h>
#include <printk.h>
#include <irqchip.h>
#include <irqdomain.h>
#include <of_address.h>

#define PRIORITY_BASE   0
#define PRIORITY_PER_ID 4

#define ENABLE_BASE     0x2000
#define ENABLE_PER_HART 0x80

#define CONTEXT_BASE        0x200000
#define CONTEXT_PER_HART    0x1000
#define CONTEXT_THRESHOLD   0x00
#define CONTEXT_CLAIM       0x04

#define PLIC_ENABLE_THRESHOLD   0

struct plic_priv {
    struct irq_domain *irqdomain;
    void *regs;
};

struct plic_handler {
    void *hart_base;
    void *enable_base;
    struct plic_priv *priv;
};

bool plic_initialized;
EXPORT_SYMBOL(plic_initialized);

static int plic_parent_irq;

static struct plic_handler plic_handler;

static inline void
plic_toggle(struct plic_handler *handler, int hwirq, int enable)
{
    u32 hwirq_mask = 1 << (hwirq % 32);
    u32 *reg = handler->enable_base + (hwirq / 32) * sizeof(u32);

    if (enable)
        writel(readl(reg) | hwirq_mask, reg);
    else
        writel(readl(reg) & ~hwirq_mask, reg);
}

static inline void
plic_irq_toggle(const struct cpumask *mask, struct irq_data *d, int enable)
{
    struct plic_priv *priv = irq_get_chip_data(d->irq);

    writel(enable, priv->regs + PRIORITY_BASE + d->hwirq * PRIORITY_PER_ID);
    plic_toggle(&plic_handler, d->hwirq, enable);
}

static int
plic_set_affinity(struct irq_data *d,
                  const struct cpumask *mask_val,
                  bool force)
{
    plic_irq_toggle(NULL, d, 0);
    plic_irq_toggle(NULL, d, 1);
    return IRQ_SET_MASK_OK_DONE;
}

static void plic_irq_eoi(struct irq_data *d)
{
    writel(d->hwirq, plic_handler.hart_base + CONTEXT_CLAIM);
}

static struct irq_chip plic_chip = {
    .name = "SiFive PLIC",
    .irq_eoi = plic_irq_eoi,
    .irq_set_affinity = plic_set_affinity,
};

static int
plic_irqdomain_map(struct irq_domain *d, unsigned int irq,
                   irq_hw_number_t hwirq)
{
    struct plic_priv *priv = d->host_data;

    irq_domain_set_info(d, irq, hwirq, &plic_chip, d->host_data,
                        handle_fasteoi_irq, NULL, NULL);

    irq_set_affinity(irq, NULL);
    return 0;
}

static int
plic_irq_domain_alloc(struct irq_domain *domain, unsigned int virq,
                      unsigned int nr_irqs, void *arg)
{
    int i, ret;
    unsigned int type;
    irq_hw_number_t hwirq;
    struct irq_fwspec *fwspec = arg;

    ret = irq_domain_translate_onecell(domain, fwspec, &hwirq, &type);
    if (ret)
        return ret;

    for (i = 0; i < nr_irqs; i++) {
        ret = plic_irqdomain_map(domain, virq + i, hwirq + i);
        if (ret)
            return ret;
    }

    return 0;
}

static const struct irq_domain_ops plic_irqdomain_ops = {
    .translate  = irq_domain_translate_onecell,
    .alloc  = plic_irq_domain_alloc,
};

static void plic_set_threshold(struct plic_handler *handler, u32 threshold)
{
    /* priority must be > threshold to trigger an interrupt */
    writel(threshold, handler->hart_base + CONTEXT_THRESHOLD);
}

static int plic_starting_cpu(void)
{
    if (plic_parent_irq)
        enable_percpu_irq(plic_parent_irq);
    else
        panic("parent irq not available");

    plic_set_threshold(&plic_handler, PLIC_ENABLE_THRESHOLD);
    return 0;
}

static void plic_handle_irq(struct irq_desc *desc)
{
    irq_hw_number_t hwirq;
    struct plic_handler *handler = &plic_handler;
    struct irq_chip *chip = irq_desc_get_chip(desc);
    void *claim = handler->hart_base + CONTEXT_CLAIM;

    while ((hwirq = readl(claim))) {
        int irq = irq_find_mapping(handler->priv->irqdomain, hwirq);

        if (unlikely(irq <= 0))
            panic("can't find mapping for hwirq %lu", hwirq);
        else
            generic_handle_irq(irq);
    }
}

static int
plic_init(struct device_node *node, struct device_node *parent)
{
    int i;
    u32 nr_irqs;
    int nr_contexts;
    struct plic_priv *priv;

    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        panic("out of memory!");

    priv->regs = of_iomap(node, 0);
    if (!priv->regs)
        panic("bad regs!");

    of_property_read_u32(node, "riscv,ndev", &nr_irqs);
    if (!nr_irqs)
        panic("out iounmap!");

    nr_contexts = of_irq_count(node);

    priv->irqdomain = irq_domain_add_linear(node, nr_irqs + 1,
                                            &plic_irqdomain_ops, priv);
    if (!priv->irqdomain)
        panic("irq domain can not add linear!");

    for (i = 0; i < nr_contexts; i++) {
        struct of_phandle_args parent;

        if (of_irq_parse_one(node, i, &parent))
            panic("failed to parse parent for context %d.", i);

        /*
         * Skip contexts other than external interrupts for our
         * privilege level.
         */
        if (parent.args[0] != RV_IRQ_EXT)
            continue;

        /* Find parent domain and register chained handler */
        if (!plic_parent_irq && irq_find_host(parent.np)) {
            plic_parent_irq = irq_of_parse_and_map(node, i);
            if (plic_parent_irq)
                irq_set_chained_handler(plic_parent_irq, plic_handle_irq);
        }

        plic_handler.hart_base =
            priv->regs + CONTEXT_BASE + i * CONTEXT_PER_HART;

        plic_handler.enable_base =
            priv->regs + ENABLE_BASE + i * ENABLE_PER_HART;

        plic_handler.priv = priv;
    }

    plic_starting_cpu();

    return 0;
}

static int
init_module(void)
{
    struct of_device_id match = {
        .compatible = "riscv,plic0",
        .data = plic_init,
    };

    printk("module[plic]: init begin ...\n");

    of_irq_init(&match);

    plic_initialized = true;

    printk("module[plic]: init end!\n");
    return 0;
}

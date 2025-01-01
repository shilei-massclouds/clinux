// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <csr.h>
#include <irq.h>
#include <export.h>
#include <printk.h>
#include <of_irq.h>
#include <irqchip.h>
#include <irqdesc.h>
#include <irqdomain.h>

bool intc_initialized;
EXPORT_SYMBOL(intc_initialized);

static struct irq_domain *intc_domain;

static void
riscv_intc_irq(struct pt_regs *regs)
{
    unsigned long cause = regs->cause & ~CAUSE_IRQ_FLAG;

    if (unlikely(cause >= BITS_PER_LONG))
        panic("unexpected interrupt cause");

    switch (cause) {
    case IRQ_S_SOFT:
        panic("no irq soft!");
        break;
    default:
        printk("%s: 1\n", __func__);
        handle_domain_irq(intc_domain, cause, regs);
        printk("%s: 2\n", __func__);
        break;
    }
}

static void riscv_intc_irq_mask(struct irq_data *d)
{
    csr_clear(CSR_IE, BIT(d->hwirq));
}

static void riscv_intc_irq_unmask(struct irq_data *d)
{
    printk("### %s ###: hwirq(0x%x) irq(0x%x) chip(%s) domain(%s)\n",
           __func__, d->hwirq, d->irq, d->chip->name, d->domain->name);

    csr_set(CSR_IE, BIT(d->hwirq));
}

static struct irq_chip riscv_intc_chip = {
    .name       = "RISC-V INTC",
    .irq_mask   = riscv_intc_irq_mask,
    .irq_unmask = riscv_intc_irq_unmask,
};

static int
riscv_intc_domain_map(struct irq_domain *d,
                      unsigned int irq, irq_hw_number_t hwirq)
{
    irq_domain_set_info(d, irq, hwirq, &riscv_intc_chip, d->host_data,
                        handle_percpu_devid_irq, NULL, NULL);

    return 0;
}

static const struct irq_domain_ops riscv_intc_domain_ops = {
    .map    = riscv_intc_domain_map,
    .xlate  = irq_domain_xlate_onecell,
};

static int
riscv_intc_init(struct device_node *node,
                struct device_node *parent)
{
    int rc;

    intc_domain = irq_domain_add_linear(node, BITS_PER_LONG,
                                        &riscv_intc_domain_ops, NULL);
    if (!intc_domain)
        panic("unable to add IRQ domain");

    rc = set_handle_irq(&riscv_intc_irq);
    if (rc)
        panic("failed to set irq handler");

    return 0;
}

int
init_module(void)
{
    struct of_device_id matchs[] = {
        { .compatible = "riscv,cpu-intc", .data = riscv_intc_init},
        {},
    };

    printk("module[intc]: init begin ...\n");

    of_irq_init(matchs);

    intc_initialized = true;

    printk("module[intc]: init end!\n");
    return 0;
}

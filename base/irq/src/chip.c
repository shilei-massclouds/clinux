// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <errno.h>
#include <export.h>
#include <irqdesc.h>

struct irq_data *irq_get_irq_data(unsigned int irq)
{
    struct irq_desc *desc = irq_to_desc(irq);

    return desc ? &desc->irq_data : NULL;
}
EXPORT_SYMBOL(irq_get_irq_data);

static void cond_unmask_eoi_irq(struct irq_desc *desc, struct irq_chip *chip)
{
    chip->irq_eoi(&desc->irq_data);
}

void handle_fasteoi_irq(struct irq_desc *desc)
{
    struct irq_chip *chip = desc->irq_data.chip;

    handle_irq_event(desc);

    cond_unmask_eoi_irq(desc, chip);
}
EXPORT_SYMBOL(handle_fasteoi_irq);

/**
 *  irq_set_chip_data - set irq chip data for an irq
 *  @irq:   Interrupt number
 *  @data:  Pointer to chip specific data
 *
 *  Set the hardware irq chip data for an irq
 */
int irq_set_chip_data(unsigned int irq, void *data)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return -EINVAL;

    desc->irq_data.chip_data = data;
    return 0;
}
EXPORT_SYMBOL(irq_set_chip_data);

void irq_percpu_enable(struct irq_desc *desc)
{
    printk("%s: desc(%p)\n", __func__, desc);
    desc->irq_data.chip->irq_unmask(&desc->irq_data);
}

void handle_percpu_devid_irq(struct irq_desc *desc)
{
    unsigned int irq = irq_desc_get_irq(desc);
    struct irqaction *action = desc->action;
    struct irq_chip *chip = irq_desc_get_chip(desc);

    printk("%s: irq(%u) (%s)\n", __func__, irq, chip->name);

    if (likely(action))
        action->handler(irq, action->percpu_dev_id);
    else
        panic("!!! NOTICE: no action for irq:%u !", irq);
}
EXPORT_SYMBOL(handle_percpu_devid_irq);

static void
__irq_do_set_handler(struct irq_desc *desc, irq_flow_handler_t handle,
                     int is_chained, const char *name)
{
    BUG_ON(!handle);

    desc->handle_irq = handle;
    desc->name = name;
}

void
__irq_set_handler(unsigned int irq, irq_flow_handler_t handle,
                  int is_chained, const char *name)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return;

    __irq_do_set_handler(desc, handle, is_chained, name);
}
EXPORT_SYMBOL(__irq_set_handler);

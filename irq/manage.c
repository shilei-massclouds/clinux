// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <irqchip.h>
#include <irqdesc.h>
#include <interrupt.h>

extern struct irq_chip no_irq_chip;

static int
__setup_irq(unsigned int irq,
            struct irq_desc *desc,
            struct irqaction *new)
{
    if (!desc)
        return -EINVAL;

    if (desc->irq_data.chip == &no_irq_chip)
        return -ENOSYS;

    new->irq = irq;
    desc->action = new;

    return 0;
}

int
request_threaded_irq(unsigned int irq,
                     irq_handler_t handler,
                     irq_handler_t thread_fn,
                     unsigned long irqflags,
                     const char *devname,
                     void *dev_id)
{
    int retval;
    struct irq_desc *desc;
    struct irqaction *action;

    desc = irq_to_desc(irq);
    if (!desc)
        return -EINVAL;

    action = kzalloc(sizeof(struct irqaction), GFP_KERNEL);
    if (!action)
        return -ENOMEM;

    action->handler = handler;
    action->thread_fn = thread_fn;
    action->flags = irqflags;
    action->name = devname;
    action->dev_id = dev_id;

    retval = __setup_irq(irq, desc, action);
    return retval;
}
EXPORT_SYMBOL(request_threaded_irq);

int
irq_do_set_affinity(struct irq_data *data,
                    const struct cpumask *mask,
                    bool force)
{
    struct irq_chip *chip = irq_data_get_irq_chip(data);

    return chip->irq_set_affinity(data, mask, force);
}

static int
irq_try_set_affinity(struct irq_data *data,
                     const struct cpumask *dest,
                     bool force)
{
    return irq_do_set_affinity(data, dest, force);
}

int
irq_set_affinity_locked(struct irq_data *data,
                        const struct cpumask *mask,
                        bool force)
{
    printk("%s: 1\n", __func__);
    return irq_try_set_affinity(data, mask, force);
}

int __irq_set_affinity(unsigned int irq,
                       const struct cpumask *mask,
                       bool force)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return -EINVAL;

    printk("%s: 1 irq(%u)\n", __func__, irq);
    printk("%s: 2 irq(%u) (%u)\n", __func__, irq, desc->irq_data.irq);
    return irq_set_affinity_locked(irq_desc_get_irq_data(desc), mask, force);
}
EXPORT_SYMBOL(__irq_set_affinity);

void enable_percpu_irq(unsigned int irq)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return;

    printk("%s: irq(%x)\n", __func__, irq);
    irq_percpu_enable(desc);
}
EXPORT_SYMBOL(enable_percpu_irq);

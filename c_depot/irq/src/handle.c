// SPDX-License-Identifier: GPL-2.0-only

#include <irq.h>
#include <errno.h>
#include <export.h>
#include <printk.h>
#include <irqdesc.h>

extern void (*handle_arch_irq)(struct pt_regs *);

int
set_handle_irq(void (*handle_irq)(struct pt_regs *))
{
    if (handle_arch_irq)
        return -EBUSY;

    handle_arch_irq = handle_irq;
    return 0;
}
EXPORT_SYMBOL(set_handle_irq);

irqreturn_t
__handle_irq_event_percpu(struct irq_desc *desc, unsigned int *flags)
{
    struct irqaction *action;
     irqreturn_t retval = IRQ_NONE;
     unsigned int irq = desc->irq_data.irq;

    for_each_action_of_desc(desc, action) {
        irqreturn_t res;

        res = action->handler(irq, action->dev_id);
        switch (res) {
        case IRQ_HANDLED:
            *flags |= action->flags;
            break;

        case IRQ_WAKE_THREAD:
            panic("bad handler IRQ_WAKE_THREAD");
            break;

        default:
            break;
        }

        retval |= res;
    }

    return retval;
}

irqreturn_t handle_irq_event_percpu(struct irq_desc *desc)
{
    unsigned int flags = 0;
    return __handle_irq_event_percpu(desc, &flags);
}

irqreturn_t handle_irq_event(struct irq_desc *desc)
{
    return handle_irq_event_percpu(desc);
}

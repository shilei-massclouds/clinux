/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_IRQDESC_H
#define _LINUX_IRQDESC_H

#include <irq.h>
#include <ptrace.h>
#include <irqdomain.h>
#include <irqhandler.h>

struct irq_desc {
    struct irq_data irq_data;
    irq_flow_handler_t handle_irq;
    struct irqaction *action;    /* IRQ action list */
    const char *name;
};

/*
 * Convert a HW interrupt number to a logical one using a IRQ domain,
 * and handle the result interrupt number. Return -EINVAL if
 * conversion failed. Providing a NULL domain indicates that the
 * conversion has already been done.
 */
int __handle_domain_irq(struct irq_domain *domain,
                        unsigned int hwirq,
                        bool lookup,
                        struct pt_regs *regs);

static inline int
handle_domain_irq(struct irq_domain *domain,
                  unsigned int hwirq,
                  struct pt_regs *regs)
{
    return __handle_domain_irq(domain, hwirq, true, regs);
}

/*
 * Architectures call this to let the generic IRQ layer
 * handle an interrupt.
 */
static inline void generic_handle_irq_desc(struct irq_desc *desc)
{
    desc->handle_irq(desc);
}

struct irq_desc *irq_to_desc(unsigned int irq);

int
__irq_alloc_descs(int irq, unsigned int from, unsigned int cnt,
                  const struct irq_affinity_desc *affinity);

static inline struct irq_data *
irq_desc_get_irq_data(struct irq_desc *desc)
{
    return &desc->irq_data;
}

static inline unsigned int irq_desc_get_irq(struct irq_desc *desc)
{
    return desc->irq_data.irq;
}

static inline struct irq_chip *irq_desc_get_chip(struct irq_desc *desc)
{
    return desc->irq_data.chip;
}

int generic_handle_irq(unsigned int irq);

#endif /* _LINUX_IRQDESC_H */

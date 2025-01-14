/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _IRQ_H_
#define _IRQ_H_

#include <types.h>
#include <ptrace.h>
#include <irqreturn.h>
#include <irqhandler.h>
#include <mod_devicetable.h>

#define NR_IRQS 64
#define IRQ_BITMAP_BITS (NR_IRQS + 8196)

#define for_each_matching_node_and_match(dn, matches, match) \
    for (dn = of_find_matching_node_and_match(NULL, matches, match); \
         dn; dn = of_find_matching_node_and_match(dn, matches, match))

enum {
    IRQ_TYPE_NONE   = 0x00000000,
};

/*
 * Return value for chip->irq_set_affinity()
 *
 * IRQ_SET_MASK_OK  - OK, core updates irq_common_data.affinity
 * IRQ_SET_MASK_NOCPY   - OK, chip did update irq_common_data.affinity
 * IRQ_SET_MASK_OK_DONE - Same as IRQ_SET_MASK_OK for core. Special code to
 *            support stacked irqchips, which indicates skipping
 *            all descendent irqchips.
 */
enum {
    IRQ_SET_MASK_OK = 0,
    IRQ_SET_MASK_OK_NOCOPY,
    IRQ_SET_MASK_OK_DONE,
};


struct irq_data {
    unsigned int irq;
    unsigned long hwirq;
    struct irq_chip *chip;
    struct irq_domain *domain;
    struct irq_data *parent_data;
    void *chip_data;
};

struct irq_chip {
    const char *name;
    void (*irq_mask)(struct irq_data *data);
    void (*irq_unmask)(struct irq_data *data);
    void (*irq_eoi)(struct irq_data *data);
    int (*irq_set_affinity)(struct irq_data *data,
                            const struct cpumask *dest,
                            bool force);
};

int set_handle_irq(void (*handle_irq)(struct pt_regs *));

struct device_node *
of_find_matching_node_and_match(struct device_node *from,
                                const struct of_device_id *matches,
                                const struct of_device_id **match);

static inline struct irq_chip *irq_data_get_irq_chip(struct irq_data *d)
{
    return d->chip;
}

struct irq_data *irq_get_irq_data(unsigned int irq);

static inline void *irq_get_chip_data(unsigned int irq)
{
    struct irq_data *d = irq_get_irq_data(irq);
    return d ? d->chip_data : NULL;
}

void enable_percpu_irq(unsigned int irq);

void
__irq_set_handler(unsigned int irq, irq_flow_handler_t handle,
                  int is_chained, const char *name);

/*
 * Set a highlevel chained flow handler for a given IRQ.
 * (a chained handler is automatically enabled and set to
 *  IRQ_NOREQUEST, IRQ_NOPROBE, and IRQ_NOTHREAD)
 */
static inline void
irq_set_chained_handler(unsigned int irq, irq_flow_handler_t handle)
{
    __irq_set_handler(irq, handle, 1, NULL);
}

irqreturn_t handle_irq_event(struct irq_desc *desc);

#define for_each_action_of_desc(desc, act) \
    for (act = desc->action; act; act = act->next)

#endif /* _IRQ_H_ */

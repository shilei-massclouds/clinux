// SPDX-License-Identifier: GPL-2.0

#include <bug.h>
#include <irq.h>
#include <slab.h>
#include <errno.h>
#include <bitmap.h>
#include <export.h>
#include <hardirq.h>
#include <irqdesc.h>
#include <irq_regs.h>
#include <radix-tree.h>

extern struct irq_chip no_irq_chip;

int nr_irqs = NR_IRQS;

static RADIX_TREE(irq_desc_tree, GFP_KERNEL);
static DECLARE_BITMAP(allocated_irqs, IRQ_BITMAP_BITS);

struct irq_desc *irq_to_desc(unsigned int irq)
{
    return radix_tree_lookup(&irq_desc_tree, irq);
}
EXPORT_SYMBOL(irq_to_desc);

int generic_handle_irq(unsigned int irq)
{
    struct irq_desc *desc = irq_to_desc(irq);

    if (!desc)
        return -EINVAL;

    generic_handle_irq_desc(desc);
    return 0;
}
EXPORT_SYMBOL(generic_handle_irq);

int
__handle_domain_irq(struct irq_domain *domain,
                    unsigned int hwirq,
                    bool lookup,
                    struct pt_regs *regs)
{
    unsigned int irq = hwirq;
    struct pt_regs *old_regs = set_irq_regs(regs);

    irq_enter();

    /*
     * Some hardware gives randomly wrong interrupts.  Rather
     * than crashing, do something sensible.
     */
    if (unlikely(!irq || irq >= nr_irqs)) {
        panic("bad irq(%u)", irq);
    } else {
        generic_handle_irq(irq);
    }

    irq_exit();
    set_irq_regs(old_regs);
    return 0;
}
EXPORT_SYMBOL(__handle_domain_irq);

static void
desc_set_defaults(unsigned int irq, struct irq_desc *desc)
{
    desc->irq_data.irq = irq;
    desc->irq_data.chip = &no_irq_chip;
    desc->irq_data.chip_data = NULL;
}

static struct irq_desc *
alloc_desc(int irq, unsigned int flags)
{
    struct irq_desc *desc;

    desc = kzalloc(sizeof(*desc), GFP_KERNEL);
    if (!desc)
        return NULL;

    desc_set_defaults(irq, desc);
    return desc;
}

static void
irq_insert_desc(unsigned int irq, struct irq_desc *desc)
{
    radix_tree_insert(&irq_desc_tree, irq, desc);
}

static int
alloc_descs(unsigned int start, unsigned int cnt,
            const struct irq_affinity_desc *affinity)
{
    int i;
    struct irq_desc *desc;

    BUG_ON(affinity);

    for (i = 0; i < cnt; i++) {
        unsigned int flags = 0;

        desc = alloc_desc(start + i, flags);
        if (!desc)
            panic("bad desc!");

        irq_insert_desc(start + i, desc);
    }
    bitmap_set(allocated_irqs, start, cnt);
    return start;
}

int
__irq_alloc_descs(int irq, unsigned int from, unsigned int cnt,
                  const struct irq_affinity_desc *affinity)
{
    int start, ret;

    if (!cnt)
        return -EINVAL;

    if (irq >= 0) {
        if (from > irq)
            return -EINVAL;
        from = irq;
    }

    start = bitmap_find_next_zero_area(allocated_irqs, IRQ_BITMAP_BITS,
                                       from, cnt, 0);

    if (irq >=0 && start != irq)
        return -EEXIST;

    if (start + cnt > nr_irqs)
        panic("need to extend!");

    return alloc_descs(start, cnt, affinity);
}
EXPORT_SYMBOL(__irq_alloc_descs);

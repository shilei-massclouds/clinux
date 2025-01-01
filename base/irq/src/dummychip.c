// SPDX-License-Identifier: GPL-2.0

#include <irq.h>

/*
 * NOP functions
 */
static void noop(struct irq_data *data) { }

static unsigned int noop_ret(struct irq_data *data)
{
    return 0;
}

/*
 * Generic no controller implementation
 */
struct irq_chip no_irq_chip = {
    .name           = "none",
};

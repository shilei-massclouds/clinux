/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __OF_IRQ_H
#define __OF_IRQ_H

#include <fdt.h>
#include <mod_devicetable.h>

typedef int (*of_irq_init_cb_t)(struct device_node *, struct device_node *);

int of_irq_get(struct device_node *dev, int index);

void of_irq_init(const struct of_device_id *matches);

int of_irq_count(struct device_node *dev);

int
of_irq_parse_one(struct device_node *device,
                 int index,
                 struct of_phandle_args *out_irq);

unsigned int irq_of_parse_and_map(struct device_node *dev, int index);

#endif /* __OF_IRQ_H */

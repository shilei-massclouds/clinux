/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __OF_ADDRESS_H
#define __OF_ADDRESS_H

#include <of.h>

int
of_address_to_resource(struct device_node *dev,
                       int index,
                       struct resource *r);

void *of_iomap(struct device_node *np, int index);

const u32 * of_get_address(struct device_node *dev,
                           int index, u64 *size, unsigned int *flags);

int of_n_addr_cells(struct device_node *np);
int of_n_size_cells(struct device_node *np);

#endif /* __OF_ADDRESS_H */

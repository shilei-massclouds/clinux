/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_FIND_BIT_H
#define __LINUX_FIND_BIT_H

unsigned long
find_next_zero_bit(const unsigned long *addr,
                   unsigned long size,
                   unsigned long offset);

unsigned long
find_next_bit(const unsigned long *addr,
              unsigned long size,
              unsigned long offset);

#endif /* __LINUX_FIND_BIT_H */

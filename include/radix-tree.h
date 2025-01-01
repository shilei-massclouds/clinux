/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _LINUX_RADIX_TREE_H
#define _LINUX_RADIX_TREE_H

#include <xarray.h>

#define radix_tree_root xarray
#define radix_tree_node xa_node

#define RADIX_TREE_INIT(name, mask) XARRAY_INIT(name, mask)

#define RADIX_TREE(name, mask) \
    struct radix_tree_root name = RADIX_TREE_INIT(name, mask)

#define RADIX_TREE_ENTRY_MASK       3UL
#define RADIX_TREE_INTERNAL_NODE    2UL

#define RADIX_TREE_MAP_SHIFT    XA_CHUNK_SHIFT
#define RADIX_TREE_MAP_SIZE     (1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK     (RADIX_TREE_MAP_SIZE-1)

static inline bool radix_tree_is_internal_node(void *ptr)
{
    return ((unsigned long)ptr & RADIX_TREE_ENTRY_MASK) ==
        RADIX_TREE_INTERNAL_NODE;
}

void *radix_tree_lookup(const struct radix_tree_root *root,
                        unsigned long index);

int
radix_tree_insert(struct radix_tree_root *root,
                  unsigned long index, void *item);

#endif /* _LINUX_RADIX_TREE_H */

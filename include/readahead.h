/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_READAHEAD_H
#define _LINUX_READAHEAD_H

#include <fs.h>

void __do_page_cache_readahead(struct address_space *, struct file *,
                               pgoff_t index, unsigned long nr_to_read,
                               unsigned long lookahead_size);

/*
 * Submit IO for the read-ahead request in file_ra_state.
 */
static inline void ra_submit(struct file_ra_state *ra,
                             struct address_space *mapping,
                             struct file *filp)
{
    __do_page_cache_readahead(mapping, filp,
                              ra->start, ra->size, ra->async_size);
}

#endif /* _LINUX_READAHEAD_H */

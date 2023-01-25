/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FILEMAP_H
#define _LINUX_FILEMAP_H

#include <fs.h>

struct page *
read_cache_page(struct address_space *mapping,
                pgoff_t index,
                int (*filler)(void *, struct page *),
                void *data);

typedef int
(*add_to_page_cache_lru_t)(struct page *page,
                           struct address_space *mapping,
                           pgoff_t offset, gfp_t gfp_mask);
extern add_to_page_cache_lru_t add_to_page_cache_lru;

void page_endio(struct page *page, bool is_write, int err);

ssize_t
generic_file_read_iter(struct kiocb *iocb, struct iov_iter *iter);

int generic_file_mmap(struct file * file, struct vm_area_struct * vma);

#endif /* _LINUX_FILEMAP_H */

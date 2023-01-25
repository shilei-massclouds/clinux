/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PAGEMAP_H
#define _LINUX_PAGEMAP_H

#include <fs.h>
#include <gfp.h>
#include <filemap.h>
#include <mm_types.h>
#include <page-flags.h>

#define FGP_ACCESSED    0x00000001
#define FGP_LOCK        0x00000002
#define FGP_CREAT       0x00000004
#define FGP_FOR_MMAP    0x00000040

#define VM_READAHEAD_PAGES  (SZ_128K / PAGE_SIZE)

struct readahead_control {
    struct file *file;
    struct address_space *mapping;
/* private: use the readahead_* accessors instead */
    pgoff_t _index;
    unsigned int _nr_pages;
    unsigned int _batch_count;
};

struct page *
pagecache_get_page(struct address_space *mapping, pgoff_t offset,
                   int fgp_flags, gfp_t cache_gfp_mask);

static inline struct page *
find_or_create_page(struct address_space *mapping,
                    pgoff_t index,
                    gfp_t gfp_mask)
{
    return pagecache_get_page(mapping, index,
                              FGP_LOCK|FGP_ACCESSED|FGP_CREAT,
                              gfp_mask);
}

static inline struct page *
__page_cache_alloc(gfp_t gfp)
{
    return alloc_pages(gfp, 0);
}

static inline void attach_page_private(struct page *page, void *data)
{
    set_page_private(page, (unsigned long)data);
    SetPagePrivate(page);
}

static inline struct page *
find_get_page_flags(struct address_space *mapping, pgoff_t offset, int fgp_flags)
{
    return pagecache_get_page(mapping, offset, fgp_flags, 0);
}

static inline unsigned long dir_pages(struct inode *inode)
{
    return (unsigned long)(inode->i_size + PAGE_SIZE - 1) >> PAGE_SHIFT;
}

static inline struct page *
read_mapping_page(struct address_space *mapping,
                  pgoff_t index, void *data)
{
    return read_cache_page(mapping, index, NULL, data);
}

static inline gfp_t mapping_gfp_mask(struct address_space *mapping)
{
    return mapping->gfp_mask;
}

/**
 * find_get_page - find and get a page reference
 * @mapping: the address_space to search
 * @offset: the page index
 *
 * Looks up the page cache slot at @mapping & @offset.  If there is a
 * page cache page, it is returned with an increased refcount.
 *
 * Otherwise, %NULL is returned.
 */
static inline struct page *
find_get_page(struct address_space *mapping, pgoff_t offset)
{
    return pagecache_get_page(mapping, offset, 0, 0);
}

static inline gfp_t
mapping_gfp_constraint(struct address_space *mapping, gfp_t gfp_mask)
{
    return mapping_gfp_mask(mapping) & gfp_mask;
}

static inline gfp_t readahead_gfp_mask(struct address_space *x)
{
    return mapping_gfp_mask(x) | __GFP_NORETRY | __GFP_NOWARN;
}

/**
 * readahead_count - The number of pages in this readahead request.
 * @rac: The readahead request.
 */
static inline unsigned int readahead_count(struct readahead_control *rac)
{
    return rac->_nr_pages;
}

static inline struct page *readahead_page(struct readahead_control *rac)
{
    struct page *page;

    BUG_ON(rac->_batch_count > rac->_nr_pages);
    rac->_nr_pages -= rac->_batch_count;
    rac->_index += rac->_batch_count;

    if (!rac->_nr_pages) {
        rac->_batch_count = 0;
        return NULL;
    }

    page = xa_load(&rac->mapping->i_pages, rac->_index);
    rac->_batch_count = 1;

    return page;
}

static inline pgoff_t
linear_page_index(struct vm_area_struct *vma, unsigned long address)
{
    pgoff_t pgoff;

    pgoff = (address - vma->vm_start) >> PAGE_SHIFT;
    pgoff += vma->vm_pgoff;
    return pgoff;
}

/*
 * Get the offset in PAGE_SIZE.
 * (TODO: hugepage should have ->index in PAGE_SIZE)
 */
static inline pgoff_t page_to_pgoff(struct page *page)
{
    return page->index;
}

#endif /* _LINUX_PAGEMAP_H */

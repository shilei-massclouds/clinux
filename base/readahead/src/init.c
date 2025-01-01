// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <export.h>
#include <printk.h>
#include <pagemap.h>
#include <readahead.h>
#include <backing-dev.h>

/*
 * Initialise a struct file's readahead state.  Assumes that the caller has
 * memset *ra to zero.
 */
void
file_ra_state_init(struct file_ra_state *ra, struct address_space *mapping)
{
    ra->ra_pages = inode_to_bdi(mapping->host)->ra_pages;
    ra->prev_pos = -1;
}
EXPORT_SYMBOL(file_ra_state_init);

static unsigned long get_init_ra_size(unsigned long size, unsigned long max)
{
    unsigned long newsize = roundup_pow_of_two(size);

    if (newsize <= max / 32)
        newsize = newsize * 4;
    else if (newsize <= max / 4)
        newsize = newsize * 2;
    else
        newsize = max;

    return newsize;
}

static void
ondemand_readahead(struct address_space *mapping,
                   struct file_ra_state *ra, struct file *filp,
                   bool hit_readahead_marker, pgoff_t index,
                   unsigned long req_size)
{
    unsigned long max_pages = ra->ra_pages;
    struct backing_dev_info *bdi = inode_to_bdi(mapping->host);

    if (req_size > max_pages && bdi->io_pages > max_pages)
        max_pages = min(req_size, bdi->io_pages);

    BUG_ON(index);

    ra->start = index;
    ra->size = get_init_ra_size(req_size, max_pages);
    ra->async_size = ra->size > req_size ? ra->size - req_size : ra->size;

    printk("%s: ra_pages(%lu) max_pages(%lu) io_pages(%lu) req_size(%lu) (%lu, %lu)\n",
           __func__, ra->ra_pages, max_pages, bdi->io_pages, req_size,
           ra->size, ra->async_size);

    ra_submit(ra, mapping, filp);
}

void
page_cache_sync_readahead(struct address_space *mapping,
                          struct file_ra_state *ra, struct file *filp,
                          pgoff_t index, unsigned long req_count)
{
    /* no read-ahead */
    if (!ra->ra_pages)
        return;

    /* do read-ahead */
    ondemand_readahead(mapping, ra, filp, false, index, req_count);
}
EXPORT_SYMBOL(page_cache_sync_readahead);

static void
read_pages(struct readahead_control *rac, struct list_head *pages,
           bool skip_page)
{
    struct page *page;
    const struct address_space_operations *aops = rac->mapping->a_ops;

    if (!readahead_count(rac))
        goto out;

    if (aops->readahead) {
        aops->readahead(rac);
        /* Clean up the remaining pages */
        while ((page = readahead_page(rac))) {
            /* Todo: unlock and put page! */
        }
    } else if (aops->readpages) {
        panic("aops no readpages!");
    } else {
        panic("no aops!");
    }

    BUG_ON(!list_empty(pages));
    BUG_ON(readahead_count(rac));

out:
    if (skip_page)
        rac->_index++;
}

void page_cache_readahead_unbounded(struct address_space *mapping,
        struct file *file, pgoff_t index, unsigned long nr_to_read,
        unsigned long lookahead_size)
{
    unsigned long i;
    LIST_HEAD(page_pool);
    gfp_t gfp_mask = readahead_gfp_mask(mapping);
    struct readahead_control rac = {
        .mapping = mapping,
        .file = file,
        ._index = index,
    };

    /*
     * Preallocate as many pages as we will need.
     */
    for (i = 0; i < nr_to_read; i++) {
        struct page *page = xa_load(&mapping->i_pages, index + i);

        BUG_ON(index + i != rac._index + rac._nr_pages);

        if (page) {
            read_pages(&rac, &page_pool, true);
            continue;
        }

        page = __page_cache_alloc(gfp_mask);
        if (!page)
            panic("out of memory!");

        if (mapping->a_ops->readpages)
            panic("need a_ops->readpages!");
        else if (add_to_page_cache_lru(page, mapping, index + i,
                                       gfp_mask) < 0)
            panic("add to page error!");

        if (i == nr_to_read - lookahead_size)
            SetPageReadahead(page);

        rac._nr_pages++;
    }

    /*
     * Now start the IO.  We ignore I/O errors - if the page is not
     * uptodate then the caller will launch readpage again, and
     * will then handle the error.
     */
    read_pages(&rac, &page_pool, false);
}

void __do_page_cache_readahead(struct address_space *mapping,
                               struct file *file, pgoff_t index,
                               unsigned long nr_to_read,
                               unsigned long lookahead_size)
{
    pgoff_t end_index;  /* The last page we want to read */
    struct inode *inode = mapping->host;
    loff_t isize = i_size_read(inode);

    if (isize == 0)
        return;

    end_index = (isize - 1) >> PAGE_SHIFT;
    if (index > end_index)
        return;

    /* Don't read past the page containing the last byte of the file */
    if (nr_to_read > end_index - index)
        nr_to_read = end_index - index + 1;

    page_cache_readahead_unbounded(mapping, file,
                                   index, nr_to_read, lookahead_size);
}
EXPORT_SYMBOL(__do_page_cache_readahead);

/**
 * page_cache_async_readahead - file readahead for marked pages
 * @mapping: address_space which holds the pagecache and I/O vectors
 * @ra: file_ra_state which holds the readahead state
 * @filp: passed on to ->readpage() and ->readpages()
 * @page: The page at @index which triggered the readahead call.
 * @index: Index of first page to be read.
 * @req_count: Total number of pages being read by the caller.
 *
 * page_cache_async_readahead() should be called when a page is used which
 * is marked as PageReadahead; this is a marker to suggest that the application
 * has used up enough of the readahead window that we should start pulling in
 * more pages.
 */
void
page_cache_async_readahead(struct address_space *mapping,
               struct file_ra_state *ra, struct file *filp,
               struct page *page, pgoff_t index,
               unsigned long req_count)
{
    /* no read-ahead */
    if (!ra->ra_pages)
        return;

    /*
     * Same bit is used for PG_readahead and PG_reclaim.
     */
    if (PageWriteback(page))
        return;

    ClearPageReadahead(page);

    /* do read-ahead */
    ondemand_readahead(mapping, ra, filp, true, index, req_count);
}
EXPORT_SYMBOL(page_cache_async_readahead);

int
init_module(void)
{
    printk("module[readahead]: init begin ...\n");
    printk("module[readahead]: init end!\n");
    return 0;
}

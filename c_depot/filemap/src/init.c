// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <types.h>
#include <export.h>
#include <printk.h>
#include <xarray.h>
#include <pagemap.h>
#include <pgalloc.h>
#include <mm_types.h>
#include <readahead.h>
#include <bitops.h>
#include <sched.h>
#include <wait.h>
#include <hash.h>
#include <page_idle.h>
#include <gfp.h>

/*
 * In order to wait for pages to become available there must be
 * waitqueues associated with pages. By using a hash table of
 * waitqueues where the bucket discipline is to maintain all
 * waiters on the same queue and wake all when any of the pages
 * become available, and for the woken contexts to check to be
 * sure the appropriate page became available, this saves space
 * at a cost of "thundering herd" phenomena during rare hash
 * collisions.
 */
#define PAGE_WAIT_TABLE_BITS 8
#define PAGE_WAIT_TABLE_SIZE (1 << PAGE_WAIT_TABLE_BITS)
static wait_queue_head_t page_wait_table[PAGE_WAIT_TABLE_SIZE];

static wait_queue_head_t *page_waitqueue(struct page *page)
{
    return &page_wait_table[hash_ptr(page, PAGE_WAIT_TABLE_BITS)];
}

static void wake_up_page_bit(struct page *page, int bit_nr)
{
    panic("%s: todo!\n", __func__);
}

static void wake_up_page(struct page *page, int bit)
{
    if (!PageWaiters(page))
        return;
    wake_up_page_bit(page, bit);
}

/*
 * A choice of three behaviors for wait_on_page_bit_common():
 */
enum behavior {
    EXCLUSIVE,  /* Hold ref to page and take the bit when woken, like
             * __lock_page() waiting on then setting PG_locked.
             */
    SHARED,     /* Hold ref to page and check the bit when woken, like
             * wait_on_page_writeback() waiting on PG_writeback.
             */
    DROP,       /* Drop ref to page before wait, no check when woken,
             * like put_and_wait_on_page_locked() on PG_locked.
             */
};

static inline int
wait_on_page_bit_common(wait_queue_head_t *q,
                        struct page *page,
                        int bit_nr,
                        int state,
                        enum behavior behavior)
{
    panic("%s: todo!\n", __func__);
}

static int
__add_to_page_cache_locked(struct page *page,
                           struct address_space *mapping,
                           pgoff_t offset, gfp_t gfp_mask)
{
    void *old;
    XA_STATE(xas, &mapping->i_pages, offset);

    page->mapping = mapping;
    page->index = offset;

    old = xas_load(&xas);
    if (old)
        panic("already exist! (%p)", old);

    xas_store(&xas, page);
    if (xas_error(&xas))
        panic("can not store!");

    mapping->nrpages++;
    return 0;
}

int
_add_to_page_cache_lru(struct page *page,
                      struct address_space *mapping,
                      pgoff_t offset,
                      gfp_t gfp_mask)
{
    int ret;

    ret = __add_to_page_cache_locked(page, mapping, offset, gfp_mask);
    BUG_ON(ret);
    BUG_ON(PageActive(page));
    //lru_cache_add(page);
    return ret;
}

struct page *
find_get_entry(struct address_space *mapping, pgoff_t offset)
{
    struct page *page;
    XA_STATE(xas, &mapping->i_pages, offset);

    page = xas_load(&xas);
    return page;
}

struct page *
pagecache_get_page(struct address_space *mapping, pgoff_t index,
                   int fgp_flags, gfp_t gfp_mask)
{
    int err;
    struct page *page;

 repeat:
    page = find_get_entry(mapping, index);
    if (xa_is_value(page))
        page = NULL;
    if (!page)
        goto no_page;

    if (fgp_flags & FGP_LOCK) {
        if (fgp_flags & FGP_NOWAIT) {
            if (!trylock_page(page)) {
                //put_page(page);
                return NULL;
            }
        } else {
            lock_page(page);
        }

        /* Has the page been truncated? */
        if (unlikely(compound_head(page)->mapping != mapping)) {
            unlock_page(page);
            //put_page(page);
            goto repeat;
        }
        BUG_ON(page->index != index);
    }

    if (fgp_flags & FGP_ACCESSED)
        mark_page_accessed(page);
    else if (fgp_flags & FGP_WRITE) {
        /* Clear idle flag for buffer write */
        if (page_is_idle(page))
            clear_page_idle(page);
    }

 no_page:
    if (!page && (fgp_flags & FGP_CREAT)) {
        if ((fgp_flags & FGP_WRITE))
            gfp_mask |= __GFP_WRITE;
        if (fgp_flags & FGP_NOFS)
            gfp_mask &= ~__GFP_FS;

        page = __page_cache_alloc(gfp_mask);
        if (!page)
            return NULL;

        if (!(fgp_flags & (FGP_LOCK | FGP_FOR_MMAP)))
            fgp_flags |= FGP_LOCK;

        /* Init accessed so avoid atomic mark_page_accessed later */
        if (fgp_flags & FGP_ACCESSED)
            __SetPageReferenced(page);

        err = add_to_page_cache_lru(page, mapping, index, gfp_mask);
        if (unlikely(err))
            panic("add page to cache lru error!");

        /*
         * add_to_page_cache_lru locks the page, and for mmap we expect
         * an unlocked page.
         */
        if (page && (fgp_flags & FGP_FOR_MMAP))
            unlock_page(page);
    }

    return page;
}
EXPORT_SYMBOL(pagecache_get_page);

static struct page *wait_on_page_read(struct page *page)
{
    if (!IS_ERR(page)) {
        while (!PageUptodate(page));
    }
    return page;
}

static struct page *
do_read_cache_page(struct address_space *mapping,
                   pgoff_t index,
                   int (*filler)(void *, struct page *),
                   void *data,
                   gfp_t gfp)
{
    int err;
    struct page *page;

    page = find_get_page(mapping, index);
    if (!page) {
        page = __page_cache_alloc(gfp);
        if (!page)
            panic("out of memory!");
        err = add_to_page_cache_lru(page, mapping, index, gfp);
        if (unlikely(err))
            panic("add to page cache error!");

        if (filler)
            err = filler(data, page);
        else
            err = mapping->a_ops->readpage(data, page);

        if (err < 0)
            panic("read page error!");

        page = wait_on_page_read(page);
        if (IS_ERR(page))
            panic("wait on page error!");

        return page;
    }

    if (PageUptodate(page))
        return page;

    panic("%s: !", __func__);
}

struct page *
read_cache_page(struct address_space *mapping,
                pgoff_t index,
                int (*filler)(void *, struct page *),
                void *data)
{
    return do_read_cache_page(mapping, index, filler, data,
                              mapping_gfp_mask(mapping));
}
EXPORT_SYMBOL(read_cache_page);

void page_endio(struct page *page, bool is_write, int err)
{
    if (!is_write) {
        if (!err) {
            SetPageUptodate(page);
        } else {
            ClearPageUptodate(page);
            SetPageError(page);
        }
        //unlock_page(page);
    } else {
        if (err) {
            panic("err: %d", err);
        }
        //end_page_writeback(page);
    }
}
EXPORT_SYMBOL(page_endio);

ssize_t generic_file_buffered_read(struct kiocb *iocb,
                                   struct iov_iter *iter,
                                   ssize_t written)
{
    pgoff_t index;
    pgoff_t last_index;
    pgoff_t prev_index;
    unsigned int prev_offset;
    unsigned long offset;      /* offset into pagecache page */
    int error = 0;
    struct file *filp = iocb->ki_filp;
    struct file_ra_state *ra = &filp->f_ra;
    struct address_space *mapping = filp->f_mapping;
    struct inode *inode = mapping->host;
    loff_t *ppos = &iocb->ki_pos;

    index = *ppos >> PAGE_SHIFT;
    prev_index = ra->prev_pos >> PAGE_SHIFT;
    prev_offset = ra->prev_pos & (PAGE_SIZE-1);
    last_index = (*ppos + iter->count + PAGE_SIZE-1) >> PAGE_SHIFT;
    offset = *ppos & ~PAGE_MASK;

    for (;;) {
        loff_t isize;
        unsigned long nr;
        unsigned long ret;
        struct page *page;
        pgoff_t end_index;

        page = find_get_page(mapping, index);
        if (!page) {
            page_cache_sync_readahead(mapping, ra, filp,
                                      index, last_index - index);
            page = find_get_page(mapping, index);
            if (unlikely(page == NULL))
                panic("no cached page!");
        }

        if (PageReadahead(page)) {
            page_cache_async_readahead(mapping, ra, filp, page,
                                       index, last_index - index);
        }
        if (!PageUptodate(page))
            while (!PageUptodate(page));

        /*
         * i_size must be checked after we know the page is Uptodate.
         *
         * Checking i_size after the check allows us to calculate
         * the correct value for "nr", which means the zero-filled
         * part of the page is not copied back to userspace (unless
         * another truncate extends the file - this is desired though).
         */
        isize = i_size_read(inode);
        end_index = (isize - 1) >> PAGE_SHIFT;
        if (unlikely(!isize || index > end_index))
            goto out;

        /* nr is the maximum number of bytes to copy from this page */
        nr = PAGE_SIZE;
        if (index == end_index) {
            nr = ((isize - 1) & ~PAGE_MASK) + 1;
            if (nr <= offset) {
                goto out;
            }
        }
        nr = nr - offset;

        prev_index = index;

        /*
         * Ok, we have the page, and it's up-to-date, so
         * now we can copy it to user space...
         */
        ret = copy_page_to_iter(page, offset, nr, iter);
        offset += ret;
        index += offset >> PAGE_SHIFT;
        offset &= ~PAGE_MASK;
        prev_offset = offset;

        written += ret;
        if (!iov_iter_count(iter))
            goto out;

        panic("copy page error!");
    }

out:
    ra->prev_pos = prev_index;
    ra->prev_pos <<= PAGE_SHIFT;
    ra->prev_pos |= prev_offset;

    *ppos = ((loff_t)index << PAGE_SHIFT) + offset;
    return written ? written : error;
}
EXPORT_SYMBOL(generic_file_buffered_read);

ssize_t
generic_file_read_iter(struct kiocb *iocb, struct iov_iter *iter)
{
    ssize_t retval = 0;
    size_t count = iov_iter_count(iter);

    if (!count)
        return 0;

    return generic_file_buffered_read(iocb, iter, retval);
}
EXPORT_SYMBOL(generic_file_read_iter);

static inline struct file *maybe_unlock_mmap_for_io(struct vm_fault *vmf,
                            struct file *fpin)
{
    int flags = vmf->flags;

    if (fpin)
        return fpin;

    /*
     * FAULT_FLAG_RETRY_NOWAIT means we don't want to wait on page locks or
     * anything, so we only pin the file and drop the mmap_lock if only
     * FAULT_FLAG_ALLOW_RETRY is set, while this is the first attempt.
     */
    return vmf->vma->vm_file;
}

static struct file *do_sync_mmap_readahead(struct vm_fault *vmf)
{
    struct file *fpin = NULL;
    pgoff_t offset = vmf->pgoff;
    struct file *file = vmf->vma->vm_file;
    struct file_ra_state *ra = &file->f_ra;
    struct address_space *mapping = file->f_mapping;

    /* If we don't want any read-ahead, don't bother */
    if (vmf->vma->vm_flags & VM_RAND_READ)
        return fpin;
    if (!ra->ra_pages)
        return fpin;

    if (vmf->vma->vm_flags & VM_SEQ_READ)
        panic("VM_SEQ_READ");

    /*
     * mmap read-around
     */
    fpin = maybe_unlock_mmap_for_io(vmf, fpin);
    ra->start = max_t(long, 0, offset - ra->ra_pages / 2);
    ra->size = ra->ra_pages;
    ra->async_size = ra->ra_pages / 4;
    ra_submit(ra, mapping, file);
    return fpin;
}

/*
 * Asynchronous readahead happens when we find the page and PG_readahead,
 * so we want to possibly extend the readahead further.  We return the file that
 * was pinned if we have to drop the mmap_lock in order to do IO.
 */
static struct file *
do_async_mmap_readahead(struct vm_fault *vmf, struct page *page)
{
    struct file *file = vmf->vma->vm_file;
    struct file_ra_state *ra = &file->f_ra;
    struct address_space *mapping = file->f_mapping;
    struct file *fpin = NULL;
    pgoff_t offset = vmf->pgoff;

    /* If we don't want any read-ahead, don't bother */
    if (vmf->vma->vm_flags & VM_RAND_READ || !ra->ra_pages)
        return fpin;

    return fpin;
    panic("%s: !", __func__);
    /*
    if (PageReadahead(page)) {
        fpin = maybe_unlock_mmap_for_io(vmf, fpin);
        page_cache_async_readahead(mapping, ra, file,
                                   page, offset, ra->ra_pages);
    }
    return fpin;
    */
}

/*
 * lock_page_maybe_drop_mmap - lock the page, possibly dropping the mmap_lock
 * @vmf - the vm_fault for this fault.
 * @page - the page to lock.
 * @fpin - the pointer to the file we may pin (or is already pinned).
 *
 * This works similar to lock_page_or_retry in that it can drop the mmap_lock.
 * It differs in that it actually returns the page locked if it returns 1 and 0
 * if it couldn't lock the page.  If we did have to drop the mmap_lock then fpin
 * will point to the pinned file and needs to be fput()'ed at a later point.
 */
static int lock_page_maybe_drop_mmap(struct vm_fault *vmf,
                                     struct page *page,
                                     struct file **fpin)
{
    if (trylock_page(page))
        return 1;

    panic("%s: todo!\n", __func__);
}

vm_fault_t filemap_fault(struct vm_fault *vmf)
{
    int error;
    pgoff_t max_off;
    struct page *page;
    vm_fault_t ret = 0;
    struct file *fpin = NULL;
    pgoff_t offset = vmf->pgoff;
    struct file *file = vmf->vma->vm_file;
    struct address_space *mapping = file->f_mapping;
    struct inode *inode = mapping->host;

    max_off = DIV_ROUND_UP(i_size_read(inode), PAGE_SIZE);
    if (unlikely(offset >= max_off))
        return VM_FAULT_SIGBUS;

    page = find_get_page(mapping, offset);
    if (likely(page) && !(vmf->flags & FAULT_FLAG_TRIED)) {
        /*
         * We found the page, so try async readahead before
         * waiting for the lock.
         */
        fpin = do_async_mmap_readahead(vmf, page);
    } else if (!page) {
        ret = VM_FAULT_MAJOR;
        fpin = do_sync_mmap_readahead(vmf);

 retry_find:
        page = pagecache_get_page(mapping, offset,
                                  FGP_CREAT|FGP_FOR_MMAP, GFP_KERNEL);
        if (!page) {
            if (fpin)
                panic("fpin NOT NULL!");

            return VM_FAULT_OOM;
        }
    }

    if (!lock_page_maybe_drop_mmap(vmf, page, &fpin))
        goto out_retry;

    /* Did it get truncated? */
    if (unlikely(compound_head(page)->mapping != mapping)) {
        unlock_page(page);
        goto retry_find;
    }
    BUG_ON(page_to_pgoff(page) != offset);

    /*
     * We have a locked page in the page cache, now we need to check
     * that it's up-to-date. If not, it is going to be due to an error.
     */
    if (unlikely(!PageUptodate(page)))
        goto page_not_uptodate;

    pr_info("%s: step1\n", __func__);
    /*
     * We've made it this far and we had to drop our mmap_lock, now is the
     * time to return to the upper layer and have it re-find the vma and
     * redo the fault.
     */
    if (fpin) {
        unlock_page(page);
        goto out_retry;
    }

    pr_info("%s: step2\n", __func__);
    /*
     * Found the page and have a reference on it.
     * We must recheck i_size under page lock.
     */
    max_off = DIV_ROUND_UP(i_size_read(inode), PAGE_SIZE);
    if (unlikely(offset >= max_off))
        panic("bad offset!");

    vmf->page = page;
    return ret | VM_FAULT_LOCKED;

 page_not_uptodate:
    /*
     * Umm, take care of errors if the page isn't up-to-date.
     * Try to re-read it _once_. We do this synchronously,
     * because there really aren't any performance issues here
     * and we need to check for errors.
     */
    ClearPageError(page);
    fpin = maybe_unlock_mmap_for_io(vmf, fpin);
    error = mapping->a_ops->readpage(file, page);
    if (!error) {
        wait_on_page_locked(page);
        if (!PageUptodate(page))
            error = -EIO;
    }
    if (fpin)
        goto out_retry;
    //put_page(page);

    if (!error || error == AOP_TRUNCATED_PAGE)
        goto retry_find;

    //shrink_readahead_size_eio(ra);
    return VM_FAULT_SIGBUS;

 out_retry:
    /*
     * We dropped the mmap_lock, we need to return to the fault handler to
     * re-find the vma and come back and find our hopefully still populated
     * page.
     */
    return ret | VM_FAULT_RETRY;
}

void filemap_map_pages(struct vm_fault *vmf,
                       pgoff_t start_pgoff, pgoff_t end_pgoff)
{
    struct page *page;
    unsigned long max_idx;
    pgoff_t last_pgoff = start_pgoff;
    struct file *file = vmf->vma->vm_file;
    struct address_space *mapping = file->f_mapping;
    XA_STATE(xas, &mapping->i_pages, start_pgoff);

    xas_for_each(&xas, page, end_pgoff) {
        if (!PageUptodate(page) || PageReadahead(page))
            continue;

        if (page->mapping != mapping || !PageUptodate(page))
            continue;

        max_idx = DIV_ROUND_UP(i_size_read(mapping->host), PAGE_SIZE);
        if (page->index >= max_idx)
            continue;

        vmf->address += (xas.xa_index - last_pgoff) << PAGE_SHIFT;
        if (vmf->pte)
            vmf->pte += xas.xa_index - last_pgoff;
        last_pgoff = xas.xa_index;
        if (alloc_set_pte(vmf, page))
            continue;
    }
}

const struct vm_operations_struct generic_file_vm_ops = {
    .fault          = filemap_fault,
    .map_pages      = filemap_map_pages,
    /*
    .page_mkwrite   = filemap_page_mkwrite,
    */
};

int generic_file_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct address_space *mapping = file->f_mapping;

    if (!mapping->a_ops->readpage)
        panic("a_ops has no readpage!");

    vma->vm_ops = &generic_file_vm_ops;
    return 0;
}
EXPORT_SYMBOL(generic_file_mmap);

/**
 * __lock_page - get a lock on the page, assuming we need to sleep to get it
 * @__page: the page to lock
 */
void __lock_page(struct page *__page)
{
    struct page *page = compound_head(__page);
    wait_queue_head_t *q = page_waitqueue(page);
    wait_on_page_bit_common(q, page, PG_locked, TASK_UNINTERRUPTIBLE,
                            EXCLUSIVE);
}
EXPORT_SYMBOL(__lock_page);

/**
 * unlock_page - unlock a locked page
 * @page: the page
 *
 * Unlocks the page and wakes up sleepers in ___wait_on_page_locked().
 * Also wakes sleepers in wait_on_page_writeback() because the wakeup
 * mechanism between PageLocked pages and PageWriteback pages is shared.
 * But that's OK - sleepers in wait_on_page_writeback() just go back to sleep.
 *
 * Note that this depends on PG_waiters being the sign bit in the byte
 * that contains PG_locked - thus the BUILD_BUG_ON(). That allows us to
 * clear the PG_locked bit and test PG_waiters at the same time fairly
 * portably (architectures that do LL/SC can test any bit, while x86 can
 * test the sign bit).
 */
void unlock_page(struct page *page)
{
    BUG_ON(PG_waiters != 7);
    page = compound_head(page);
    BUG_ON(!PageLocked(page));
    if (clear_bit_unlock_is_negative_byte(PG_locked, &page->flags))
        wake_up_page_bit(page, PG_locked);
}
EXPORT_SYMBOL(unlock_page);

void wait_on_page_bit(struct page *page, int bit_nr)
{
    wait_queue_head_t *q = page_waitqueue(page);
    wait_on_page_bit_common(q, page, bit_nr, TASK_UNINTERRUPTIBLE, SHARED);
}
EXPORT_SYMBOL(wait_on_page_bit);

int
init_module(void)
{
    printk("module[filemap]: init begin ...\n");

    add_to_page_cache_lru = _add_to_page_cache_lru;

    printk("module[filemap]: init end!\n");

    return 0;
}

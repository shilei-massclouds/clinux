// SPDX-License-Identifier: GPL-2.0-only

#include <bio.h>
#include <slab.h>
#include <block.h>
#include <errno.h>
#include <sched.h>
#include <export.h>
#include <pagemap.h>
#include <wait_bit.h>
#include <blk_types.h>
#include <writeback.h>
#include <buffer_head.h>

#define BH_LRU_SIZE 16

struct bh_lru {
    struct buffer_head *bhs[BH_LRU_SIZE];
};

static struct bh_lru bh_lrus = {{ NULL }};

/*
 * Buffer-head allocation
 */
static struct kmem_cache *bh_cachep;

/*
 * Look up the bh in this cpu's LRU. If it's there, move it to the head.
 */
static struct buffer_head *
lookup_bh_lru(struct block_device *bdev, sector_t block, unsigned size)
{
    unsigned int i;
    struct buffer_head *ret = NULL;

    for (i = 0; i < BH_LRU_SIZE; i++) {
        struct buffer_head *bh = bh_lrus.bhs[i];

        if (bh && bh->b_blocknr == block &&
            bh->b_bdev == bdev && bh->b_size == size) {
            if (i) {
                while (i) {
                    bh_lrus.bhs[i] = bh_lrus.bhs[i - 1];
                    i--;
                }
                bh_lrus.bhs[0] = bh;
            }
            ret = bh;
            break;
        }
    }
    return ret;
}

inline void touch_buffer(struct buffer_head *bh)
{
    /* Todo: add access mark. */
}
EXPORT_SYMBOL(touch_buffer);

static struct buffer_head *
__find_get_block_slow(struct block_device *bdev, sector_t block)
{
    pgoff_t index;
    struct page *page;
    struct buffer_head *bh;
    struct buffer_head *head;
    struct inode *bd_inode = bdev->bd_inode;
    struct address_space *bd_mapping = bd_inode->i_mapping;

    index = block >> (PAGE_SHIFT - bd_inode->i_blkbits);
    page = find_get_page_flags(bd_mapping, index, FGP_ACCESSED);
    if (!page)
        return NULL;

    if (!page_has_buffers(page))
        return NULL;

    head = page_buffers(page);
    bh = head;
    do {
        if (!buffer_mapped(bh)) {
            panic("%s: all_mapped", __func__);
        } else if (bh->b_blocknr == block) {
            return bh;
        }
        bh = bh->b_this_page;
    } while (bh != head);

    panic("%s: !", __func__);
}

static void bh_lru_install(struct buffer_head *bh)
{
    int i;
    struct bh_lru *b;
    struct buffer_head *evictee = bh;

    b = &bh_lrus;
    for (i = 0; i < BH_LRU_SIZE; i++) {
        swap(evictee, b->bhs[i]);
        if (evictee == bh)
            return;
    }
}

struct buffer_head *
__find_get_block(struct block_device *bdev, sector_t block, unsigned size)
{
    struct buffer_head *bh = lookup_bh_lru(bdev, block, size);

    if (bh == NULL) {
        /* __find_get_block_slow will mark the page accessed */
        bh = __find_get_block_slow(bdev, block);
        if (bh)
            bh_lru_install(bh);
    } else {
        touch_buffer(bh);
    }

    return bh;
}
EXPORT_SYMBOL(__find_get_block);

struct buffer_head *
alloc_buffer_head(gfp_t gfp_flags)
{
    struct buffer_head *ret = kmem_cache_zalloc(bh_cachep, gfp_flags);
    if (ret)
        INIT_LIST_HEAD(&ret->b_assoc_buffers);
    return ret;
}
EXPORT_SYMBOL(alloc_buffer_head);

void set_bh_page(struct buffer_head *bh,
                 struct page *page,
                 unsigned long offset)
{
    bh->b_page = page;
    BUG_ON(offset >= PAGE_SIZE);
    bh->b_data = page_address(page) + offset;
}
EXPORT_SYMBOL(set_bh_page);

struct buffer_head *
alloc_page_buffers(struct page *page, unsigned long size, bool retry)
{
    long offset;
    struct buffer_head *bh, *head;
    gfp_t gfp = GFP_NOFS | __GFP_ACCOUNT;

    if (retry)
        gfp |= __GFP_NOFAIL;

    head = NULL;
    offset = PAGE_SIZE;
    while ((offset -= size) >= 0) {
        bh = alloc_buffer_head(gfp);
        if (!bh)
            panic("no grow!");

        bh->b_this_page = head;
        bh->b_blocknr = -1;
        head = bh;

        bh->b_size = size;

        /* Link the buffer to its page */
        set_bh_page(bh, page, offset);
    }
    return head;
}

static inline void
link_dev_buffers(struct page *page, struct buffer_head *head)
{
    struct buffer_head *bh, *tail;

    bh = head;
    do {
        tail = bh;
        bh = bh->b_this_page;
    } while (bh);
    tail->b_this_page = head;
    attach_page_private(page, head);
}

static sector_t
blkdev_max_block(struct block_device *bdev, unsigned int size)
{
    sector_t retval = ~((sector_t)0);
    loff_t sz = i_size_read(bdev->bd_inode);

    if (sz) {
        unsigned int sizebits = blksize_bits(size);
        retval = (sz >> sizebits);
    }
    return retval;
}

/*
 * Initialise the state of a blockdev page's buffers.
 */
static sector_t
init_page_buffers(struct page *page, struct block_device *bdev,
                  sector_t block, int size)
{
    int uptodate = PageUptodate(page);
    struct buffer_head *head = page_buffers(page);
    struct buffer_head *bh = head;
    sector_t end_block = blkdev_max_block(I_BDEV(bdev->bd_inode), size);

    do {
        if (!buffer_mapped(bh)) {
            bh->b_end_io = NULL;
            bh->b_private = NULL;
            bh->b_bdev = bdev;
            bh->b_blocknr = block;
            if (uptodate)
                set_buffer_uptodate(bh);
            if (block < end_block)
                set_buffer_mapped(bh);
        }
        block++;
        bh = bh->b_this_page;
    } while (bh != head);

    /*
     * Caller needs to validate requested block against end of device.
     */
    return end_block;
}

static int
grow_dev_page(struct block_device *bdev,
              sector_t block, pgoff_t index, int size, int sizebits,
              gfp_t gfp)
{
    int ret = 0;
    gfp_t gfp_mask;
    struct page *page;
    sector_t end_block;
    struct buffer_head *bh;
    struct inode *inode = bdev->bd_inode;

    gfp_mask = gfp;
    gfp_mask |= __GFP_NOFAIL;

    page = find_or_create_page(inode->i_mapping, index, gfp_mask);
    BUG_ON(page == NULL);

    BUG_ON(page_has_buffers(page));

    /*
     * Allocate some buffers for this page
     */
    bh = alloc_page_buffers(page, size, true);

    link_dev_buffers(page, bh);

    end_block = init_page_buffers(page, bdev,
                                  (sector_t)index << sizebits, size);

    ret = (block < end_block) ? 1 : -ENXIO;
    BUG_ON(ret < 0);
    return ret;
}

static int
grow_buffers(struct block_device *bdev, sector_t block, int size, gfp_t gfp)
{
    pgoff_t index;
    int sizebits;

    sizebits = -1;
    do {
        sizebits++;
    } while ((size << sizebits) < PAGE_SIZE);

    index = block >> sizebits;

    /* Create a page with the proper size buffers.. */
    return grow_dev_page(bdev, block, index, size, sizebits, gfp);
}

static struct buffer_head *
__getblk_slow(struct block_device *bdev,
              sector_t block, unsigned size, gfp_t gfp)
{
    /* Size must be multiple of hard sectorsize */
    if (size & (bdev_logical_block_size(bdev)-1) ||
        (size < 512 || size > PAGE_SIZE))
        panic("getblk(): invalid block size %d requested\n", size);

    for (;;) {
        struct buffer_head *bh;
        int ret;

        bh = __find_get_block(bdev, block, size);
        if (bh)
            return bh;

        ret = grow_buffers(bdev, block, size, gfp);
        if (ret < 0)
            return NULL;
    }
}

struct buffer_head *
__getblk_gfp(struct block_device *bdev,
             sector_t block, unsigned size, gfp_t gfp)
{
    struct buffer_head *bh = __find_get_block(bdev, block, size);

    if (bh == NULL)
        bh = __getblk_slow(bdev, block, size, gfp);
    return bh;
}
EXPORT_SYMBOL(__getblk_gfp);

static void
__end_buffer_read_notouch(struct buffer_head *bh, int uptodate)
{
    if (uptodate) {
        set_buffer_uptodate(bh);
    } else {
        /* This happens, due to failed read-ahead attempts. */
        clear_buffer_uptodate(bh);
    }
    unlock_buffer(bh);
}

/*
 * Default synchronous end-of-IO handler..  Just mark it up-to-date and
 * unlock the buffer. This is what ll_rw_block uses too.
 */
void end_buffer_read_sync(struct buffer_head *bh, int uptodate)
{
    __end_buffer_read_notouch(bh, uptodate);
}
EXPORT_SYMBOL(end_buffer_read_sync);

static void end_bio_bh_io_sync(struct bio *bio)
{
    struct buffer_head *bh = bio->bi_private;

    bh->b_end_io(bh, !bio->bi_status);
}

static int
submit_bh_wbc(int op, int op_flags, struct buffer_head *bh,
              enum rw_hint write_hint, struct writeback_control *wbc)
{
    struct bio *bio;

    BUG_ON(wbc);

    bio = bio_alloc(GFP_NOIO, 1);
    bio->bi_iter.bi_sector = bh->b_blocknr * (bh->b_size >> 9);
    bio_set_dev(bio, bh->b_bdev);
    bio_add_page(bio, bh->b_page, bh->b_size, bh_offset(bh));
    BUG_ON(bio->bi_iter.bi_size != bh->b_size);

    bio->bi_end_io = end_bio_bh_io_sync;
    bio->bi_private = bh;

    bio_set_op_attrs(bio, op, op_flags);

    submit_bio(bio);
    return 0;
}

int submit_bh(int op, int op_flags, struct buffer_head *bh)
{
    return submit_bh_wbc(op, op_flags, bh, 0, NULL);
}
EXPORT_SYMBOL(submit_bh);

static struct buffer_head *__bread_slow(struct buffer_head *bh)
{
    lock_buffer(bh);
    if (buffer_uptodate(bh)) {
        unlock_buffer(bh);
        return bh;
    } else {
        bh->b_end_io = end_buffer_read_sync;
        printk("%s: \n", __func__);
        submit_bh(REQ_OP_READ, 0, bh);
        wait_on_buffer(bh);
        printk("%s: 1 buffer_uptodate(%d)\n",
               __func__, buffer_uptodate(bh));
        if (buffer_uptodate(bh))
            return bh;
        panic("%s: !\n", __func__);
    }
    return NULL;
}

struct buffer_head *
__bread_gfp(struct block_device *bdev,
            sector_t block,
            unsigned size,
            gfp_t gfp)
{
    struct buffer_head *bh = __getblk_gfp(bdev, block, size, gfp);

    if (likely(bh) && !buffer_uptodate(bh))
        bh = __bread_slow(bh);
    return bh;
}
EXPORT_SYMBOL(__bread_gfp);

void __lock_buffer(struct buffer_head *bh)
{
    wait_on_bit_lock_io(&bh->b_state, BH_Lock, TASK_UNINTERRUPTIBLE);
}
EXPORT_SYMBOL(__lock_buffer);

void unlock_buffer(struct buffer_head *bh)
{
    clear_bit_unlock(BH_Lock, &bh->b_state);
    /* Todo */
    // wake_up_bit(&bh->b_state, BH_Lock);
}
EXPORT_SYMBOL(unlock_buffer);

/*
 * Block until a buffer comes unlocked.  This doesn't stop it
 * from becoming locked again - you have to lock it yourself
 * if you want to preserve its state.
 */
void __wait_on_buffer(struct buffer_head * bh)
{
    wait_on_bit_io(&bh->b_state, BH_Lock, TASK_UNINTERRUPTIBLE);
}
EXPORT_SYMBOL(__wait_on_buffer);

void buffer_init(void)
{
    bh_cachep = kmem_cache_create("buffer_head",
                                  sizeof(struct buffer_head), 0,
                                  (SLAB_RECLAIM_ACCOUNT|SLAB_PANIC|
                                   SLAB_MEM_SPREAD),
                                  NULL);
}

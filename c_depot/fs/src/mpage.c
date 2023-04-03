// SPDX-License-Identifier: GPL-2.0

#include <mm.h>
#include <fs.h>
#include <bio.h>
#include <block.h>
#include <blkdev.h>
#include <export.h>
#include <fs/ext2.h>
#include <pagemap.h>
#include <page-flags.h>
#include <buffer_head.h>
#include <highmem.h>

struct mpage_readpage_args {
    struct bio *bio;
    struct page *page;
    unsigned int nr_pages;
    bool is_readahead;
    sector_t last_block_in_bio;
    struct buffer_head map_bh;
    unsigned long first_logical_block;
    get_block_t *get_block;
};

static struct bio *
mpage_alloc(struct block_device *bdev,
            sector_t first_sector, int nr_vecs, gfp_t gfp_flags)
{
    struct bio *bio;

    /* Restrict the given (page cache) mask for slab allocations */
    gfp_flags &= GFP_KERNEL;
    bio = bio_alloc(gfp_flags, nr_vecs);

    if (bio == NULL) {
        while (!bio && (nr_vecs /= 2))
            bio = bio_alloc(gfp_flags, nr_vecs);
    }

    if (bio) {
        bio_set_dev(bio, bdev);
        bio->bi_iter.bi_sector = first_sector;
    }
    return bio;
}

static void mpage_end_io(struct bio *bio)
{
    struct bio_vec *bv;
    struct bvec_iter_all iter_all;

    bio_for_each_segment_all(bv, bio, iter_all) {
        struct page *page = bv->bv_page;
        page_endio(page, bio_op(bio),
                   blk_status_to_errno(bio->bi_status));
    }
}

static struct bio *mpage_bio_submit(int op, int op_flags, struct bio *bio)
{
    bio->bi_end_io = mpage_end_io;
    bio_set_op_attrs(bio, op, op_flags);
    submit_bio(bio);
    return NULL;
}

static struct bio *do_mpage_readpage(struct mpage_readpage_args *args)
{
    gfp_t gfp;
    int op_flags;
    unsigned nblocks;
    unsigned page_block;
    sector_t block_in_file;
    sector_t last_block;
    sector_t last_block_in_file;
    sector_t blocks[MAX_BUF_PER_PAGE];
    unsigned relative_block;
    int length;
    int fully_mapped = 1;
    struct page *page = args->page;
    struct inode *inode = page->mapping->host;
    const unsigned blkbits = inode->i_blkbits;
    const unsigned blocksize = 1 << blkbits;
    const unsigned blocks_per_page = PAGE_SIZE >> blkbits;
    struct buffer_head *map_bh = &args->map_bh;
    struct block_device *bdev = NULL;
    unsigned first_hole = blocks_per_page;

    if (args->is_readahead) {
        op_flags = REQ_RAHEAD;
        gfp = readahead_gfp_mask(page->mapping);
    } else {
        op_flags = 0;
        gfp = mapping_gfp_constraint(page->mapping, GFP_KERNEL);
    }

    if (page_has_buffers(page))
        panic("confused!");

    block_in_file = (sector_t)page->index << (PAGE_SHIFT - blkbits);
    last_block = block_in_file + args->nr_pages * blocks_per_page;
    last_block_in_file = (i_size_read(inode) + blocksize - 1) >> blkbits;
    if (last_block > last_block_in_file)
        last_block = last_block_in_file;
    page_block = 0;

    /*
     * Map blocks by the result from the previous get_blocks call first.
     */
    nblocks = map_bh->b_size >> blkbits;
    if (buffer_mapped(map_bh) &&
        block_in_file > args->first_logical_block &&
        block_in_file < (args->first_logical_block + nblocks)) {
        unsigned map_offset = block_in_file - args->first_logical_block;
        unsigned last = nblocks - map_offset;

        for (relative_block = 0; ; relative_block++) {
            if (relative_block == last) {
                clear_buffer_mapped(map_bh);
                break;
            }
            if (page_block == blocks_per_page)
                break;
            blocks[page_block] =
                map_bh->b_blocknr + map_offset + relative_block;

            page_block++;
            block_in_file++;
        }
        bdev = map_bh->b_bdev;
    }

    pr_debug("%s: step1 first_hole(%u)\n", __func__, first_hole);

    /*
     * Then do more get_blocks calls until we are done with this page.
     */
    map_bh->b_page = page;
    while (page_block < blocks_per_page) {
        map_bh->b_state = 0;
        map_bh->b_size = 0;

        if (block_in_file < last_block) {
            map_bh->b_size = (last_block-block_in_file) << blkbits;
            if (args->get_block(inode, block_in_file, map_bh, 0))
                panic("confused!");
            args->first_logical_block = block_in_file;
        }

        if (!buffer_mapped(map_bh)) {
            fully_mapped = 0;
            if (first_hole == blocks_per_page)
                first_hole = page_block;
            page_block++;
            block_in_file++;
            continue;
        }

        /* some filesystems will copy data into the page during
         * the get_block call, in which case we don't want to
         * read it again.  map_buffer_to_page copies the data
         * we just collected from get_block into the page's buffers
         * so readpage doesn't have to repeat the get_block call
         */
        if (buffer_uptodate(map_bh))
            panic("confused!");

        if (first_hole != blocks_per_page)
            panic("confused!");

        /* Contiguous blocks? */
        if (page_block && blocks[page_block-1] != map_bh->b_blocknr-1)
            panic("confused!");
        nblocks = map_bh->b_size >> blkbits;
        for (relative_block = 0; ; relative_block++) {
            if (relative_block == nblocks) {
                clear_buffer_mapped(map_bh);
                break;
            } else if (page_block == blocks_per_page)
                break;
            blocks[page_block] = map_bh->b_blocknr + relative_block;
            page_block++;
            block_in_file++;
        }
        bdev = map_bh->b_bdev;
    }

    if (first_hole != blocks_per_page) {
        zero_user_segment(page, first_hole << blkbits, PAGE_SIZE);
        if (first_hole == 0) {
            SetPageUptodate(page);
            //unlock_page(page);
            goto out;
        }
    } else if (fully_mapped) {
        SetPageMappedToDisk(page);
    }

    if (fully_mapped && blocks_per_page == 1 && !PageUptodate(page)) {
        SetPageUptodate(page);
        panic("confused!");
    }

    /*
     * This page will go to BIO.  Do we need to send this BIO off first?
     */
    if (args->bio && (args->last_block_in_bio != blocks[0] - 1))
        args->bio = mpage_bio_submit(REQ_OP_READ, op_flags, args->bio);

    if (args->bio == NULL) {
        if (first_hole == blocks_per_page) {
            if (!bdev_read_page(bdev, blocks[0] << (blkbits - 9), page))
                return args->bio;
        }

        args->bio = mpage_alloc(bdev, blocks[0] << (blkbits - 9),
                                min_t(int, args->nr_pages, BIO_MAX_PAGES),
                                gfp);
        if (args->bio == NULL)
            panic("confused!");
    }

    length = first_hole << blkbits;
    if (bio_add_page(args->bio, page, length, 0) < length)
        panic("bio add page error!");

    relative_block = block_in_file - args->first_logical_block;
    nblocks = map_bh->b_size >> blkbits;
    if ((buffer_boundary(map_bh) && relative_block == nblocks) ||
        (first_hole != blocks_per_page)) {
        args->bio = mpage_bio_submit(REQ_OP_READ, op_flags, args->bio);
    } else {
        args->last_block_in_bio = blocks[blocks_per_page - 1];
    }

 out:
    return args->bio;
}

int mpage_readpage(struct page *page, get_block_t get_block)
{
    struct mpage_readpage_args args = {
        .page = page,
        .nr_pages = 1,
        .get_block = get_block,
    };

    args.bio = do_mpage_readpage(&args);
    if (args.bio)
        mpage_bio_submit(REQ_OP_READ, 0, args.bio);
    return 0;
}
EXPORT_SYMBOL(mpage_readpage);

void mpage_readahead(struct readahead_control *rac, get_block_t get_block)
{
    struct page *page;
    struct mpage_readpage_args args = {
        .get_block = get_block,
        .is_readahead = true,
    };

    while ((page = readahead_page(rac))) {
        args.page = page;
        args.nr_pages = readahead_count(rac);
        args.bio = do_mpage_readpage(&args);
    }
    if (args.bio)
        mpage_bio_submit(REQ_OP_READ, REQ_RAHEAD, args.bio);
}
EXPORT_SYMBOL(mpage_readahead);

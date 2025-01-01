/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_BUFFER_HEAD_H
#define _LINUX_BUFFER_HEAD_H

#include <fs.h>
#include <gfp.h>

#define MAX_BUF_PER_PAGE (PAGE_SIZE / 512)

#define bh_offset(bh)   ((unsigned long)(bh)->b_data & ~PAGE_MASK)

enum bh_state_bits {
    BH_Uptodate,    /* Contains valid data */
    BH_Dirty,   /* Is dirty */
    BH_Lock,    /* Is locked */
    BH_Req,     /* Has been submitted for I/O */

    BH_Mapped,  /* Has a disk mapping */
    BH_New,     /* Disk mapping was newly created by get_block */
    BH_Async_Read,  /* Is under end_buffer_async_read I/O */
    BH_Async_Write, /* Is under end_buffer_async_write I/O */
    BH_Delay,   /* Buffer is not yet allocated on disk */
    BH_Boundary,    /* Block is followed by a discontiguity */
    BH_Write_EIO,   /* I/O error on write */
    BH_Unwritten,   /* Buffer is allocated on disk but not written */
    BH_Quiet,   /* Buffer Error Prinks to be quiet */
    BH_Meta,    /* Buffer contains metadata */
    BH_Prio,    /* Buffer should be submitted with REQ_PRIO */
    BH_Defer_Completion, /* Defer AIO completion to workqueue */

    BH_PrivateStart,/* not a state bit, but the first bit available
             * for private allocation by other entities
             */
};

struct buffer_head;
typedef void (bh_end_io_t)(struct buffer_head *bh, int uptodate);

struct buffer_head {
    /* circular list of page's buffers */
    struct buffer_head *b_this_page;

    unsigned long b_state;  /* buffer state bitmap (see above) */
    struct page *b_page;    /* the page this bh is mapped to */
    sector_t b_blocknr;     /* start block number */
    size_t b_size;          /* size of mapping */
    char *b_data;           /* pointer to data within the page */

    struct block_device *b_bdev;
    bh_end_io_t *b_end_io;  /* I/O completion */
    void *b_private;        /* reserved for b_end_io */

    /* associated with another mapping */
    struct list_head b_assoc_buffers;
};

/*
 * macro tricks to expand the set_buffer_foo(), clear_buffer_foo()
 * and buffer_foo() functions.
 * To avoid reset buffer flags that are already set, because that causes
 * a costly cache line transition, check the flag first.
 */
#define BUFFER_FNS(bit, name)                       \
static __always_inline void set_buffer_##name(struct buffer_head *bh)   \
{                                   \
    if (!test_bit(BH_##bit, &(bh)->b_state))            \
        set_bit(BH_##bit, &(bh)->b_state);          \
}                                   \
static __always_inline void clear_buffer_##name(struct buffer_head *bh) \
{                                   \
    clear_bit(BH_##bit, &(bh)->b_state);                \
}                                   \
static __always_inline int buffer_##name(const struct buffer_head *bh)  \
{                                   \
    return test_bit(BH_##bit, &(bh)->b_state);          \
}

BUFFER_FNS(Uptodate, uptodate)
BUFFER_FNS(Lock, locked)
BUFFER_FNS(Mapped, mapped)
BUFFER_FNS(New, new)
BUFFER_FNS(Boundary, boundary)

/* If we *know* page->private refers to buffer_heads */
#define page_buffers(page)      \
({                              \
    BUG_ON(!PagePrivate(page)); \
    ((struct buffer_head *)page_private(page)); \
})

#define page_has_buffers(page)  PagePrivate(page)

struct buffer_head *
__bread_gfp(struct block_device *bdev,
            sector_t block, unsigned size, gfp_t gfp);

struct super_block;

static inline struct buffer_head *
sb_bread(struct super_block *sb, sector_t block)
{
    return __bread_gfp(sb->s_bdev, block, sb->s_blocksize, __GFP_MOVABLE);
}

static inline struct buffer_head *
sb_bread_unmovable(struct super_block *sb, sector_t block)
{
    printk("### %s: (%u)\n", __func__, block);
    return __bread_gfp(sb->s_bdev, block, sb->s_blocksize, 0);
}

static inline int trylock_buffer(struct buffer_head *bh)
{
    return likely(!test_and_set_bit_lock(BH_Lock, &bh->b_state));
}

void __lock_buffer(struct buffer_head *bh);
void unlock_buffer(struct buffer_head *bh);

static inline void lock_buffer(struct buffer_head *bh)
{
    if (!trylock_buffer(bh))
        __lock_buffer(bh);
}

void __wait_on_buffer(struct buffer_head * bh);

static inline void wait_on_buffer(struct buffer_head *bh)
{
    if (buffer_locked(bh))
        __wait_on_buffer(bh);
}

static inline void
map_bh(struct buffer_head *bh, struct super_block *sb, sector_t block)
{
    set_buffer_mapped(bh);
    bh->b_bdev = sb->s_bdev;
    bh->b_blocknr = block;
    bh->b_size = sb->s_blocksize;
}

#endif /* _LINUX_BUFFER_HEAD_H */

// SPDX-License-Identifier: GPL-2.0-only

#include <bio.h>
#include <bvec.h>
#include <slab.h>
#include <errno.h>
#include <printk.h>
#include <string.h>
#include <export.h>
#include <mempool.h>

/*
 * Test patch to inline a certain number of bi_io_vec's inside the bio
 * itself, to shrink a bio data allocation from two mempool calls to one
 */
#define BIO_INLINE_VECS     4

/*
 * if you change this list, also change bvec_alloc or things will
 * break badly! cannot be bigger than what you can fit into an
 * unsigned short
 */
#define BV(x, n) { .nr_vecs = x, .name = "biovec-"#n }
static struct biovec_slab bvec_slabs[BVEC_POOL_NR] = {
    BV(1, 1), BV(4, 4), BV(16, 16), BV(64, 64), BV(128, 128),
    BV(BIO_MAX_PAGES, max),
};
#undef BV

/*
 * fs_bio_set is the bio_set containing bio and
 * iovec memory pools used by IO code
 * that does not need private memory pools.
 */
struct bio_set fs_bio_set;
EXPORT_SYMBOL(fs_bio_set);

struct bio_slab {
    struct kmem_cache *slab;
    unsigned int slab_size;
    char name[8];
};

void bio_init(struct bio *bio, struct bio_vec *table,
              unsigned short max_vecs)
{
    memset(bio, 0, sizeof(*bio));

    bio->bi_io_vec = table;
    bio->bi_max_vecs = max_vecs;
}
EXPORT_SYMBOL(bio_init);

struct bio_vec *
bvec_alloc(gfp_t gfp_mask, int nr, unsigned long *idx, mempool_t *pool)
{
    struct bio_vec *bvl;

    /*
     * see comment near bvec_array define!
     */
    switch (nr) {
    case 1:
        *idx = 0;
        break;
    case 2 ... 4:
        *idx = 1;
        break;
    case 5 ... 16:
        *idx = 2;
        break;
    case 17 ... 64:
        *idx = 3;
        break;
    case 65 ... 128:
        *idx = 4;
        break;
    case 129 ... BIO_MAX_PAGES:
        *idx = 5;
        break;
    default:
        return NULL;
    }

    if (*idx == BVEC_POOL_MAX) {
        panic("BVEC_POOL_MAX!");
    } else {
        struct biovec_slab *bvs = bvec_slabs + *idx;
        gfp_t __gfp_mask = gfp_mask & ~(__GFP_DIRECT_RECLAIM | __GFP_IO);

        /*
         * Make this allocation restricted and don't dump info on
         * allocation failures, since we'll fallback to the mempool
         * in case of failure.
         */
        __gfp_mask |= __GFP_NOMEMALLOC | __GFP_NORETRY | __GFP_NOWARN;

        /*
         * Try a slab allocation. If this fails and __GFP_DIRECT_RECLAIM
         * is set, retry with the 1-entry mempool
         */
        bvl = kmem_cache_alloc(bvs->slab, __gfp_mask);
        if (unlikely(!bvl && (gfp_mask & __GFP_DIRECT_RECLAIM)))
            panic("bad bvl!");

    }

    (*idx)++;
    return bvl;
}

struct bio *
bio_alloc_bioset(gfp_t gfp_mask,
                 unsigned int nr_iovecs,
                 struct bio_set *bs)
{
    void *p;
    struct bio *bio;
    unsigned front_pad;
    unsigned inline_vecs;
    struct bio_vec *bvl = NULL;
    gfp_t saved_gfp = gfp_mask;

    printk("%s: bs(0x%p) nr_iovecs(%u)\n", __func__, bs, nr_iovecs);

    if (!bs) {
        panic("bs is NULL!");
    } else {
        p = kmem_cache_alloc(bs->bio_slab, gfp_mask);
        front_pad = bs->front_pad;
        inline_vecs = BIO_INLINE_VECS;
    }

    if (unlikely(!p))
        return NULL;

    bio = p + front_pad;
    bio_init(bio, NULL, 0);

    if (nr_iovecs > inline_vecs) {
        unsigned long idx = 0;

        bvl = bvec_alloc(gfp_mask, nr_iovecs, &idx, &bs->bvec_pool);
        if (!bvl && gfp_mask != saved_gfp) {
            panic("%s: 1", __func__);
            /*
            punt_bios_to_rescuer(bs);
            gfp_mask = saved_gfp;
            bvl = bvec_alloc(gfp_mask, nr_iovecs, &idx, &bs->bvec_pool);
            */
        }

        if (unlikely(!bvl))
            panic("bad bvl!");

        bio->bi_flags |= idx << BVEC_POOL_OFFSET;
    } else if (nr_iovecs) {
        bvl = bio->bi_inline_vecs;
    }

    bio->bi_pool = bs;
    bio->bi_max_vecs = nr_iovecs;
    bio->bi_io_vec = bvl;
    return bio;
}
EXPORT_SYMBOL(bio_alloc_bioset);

/*
 * create memory pools for biovec's in a bio_set.
 * use the global biovec slabs created for general use.
 */
int biovec_init_pool(mempool_t *pool, int pool_entries)
{
    struct biovec_slab *bp = bvec_slabs + BVEC_POOL_MAX;

    return mempool_init_slab_pool(pool, pool_entries, bp->slab);
}

int
bioset_init(struct bio_set *bs,
            unsigned int pool_size,
            unsigned int front_pad,
            int flags)
{
    unsigned int back_pad = BIO_INLINE_VECS * sizeof(struct bio_vec);
    unsigned int extra_size = front_pad + back_pad;
    unsigned int sz = sizeof(struct bio) + extra_size;

    bs->front_pad = front_pad;

    bs->bio_slab = kmem_cache_create("bio-0", sz, ARCH_KMALLOC_MINALIGN,
                                     SLAB_HWCACHE_ALIGN, NULL);
    if (!bs->bio_slab)
        return -ENOMEM;

    biovec_init_pool(&bs->bvec_pool, pool_size);
    return 0;
}

void
__bio_add_page(struct bio *bio, struct page *page,
               unsigned int len, unsigned int off)
{
    struct bio_vec *bv = &bio->bi_io_vec[bio->bi_vcnt];

    BUG_ON(bio_full(bio, len));

    bv->bv_page = page;
    bv->bv_offset = off;
    bv->bv_len = len;

    bio->bi_iter.bi_size += len;
    bio->bi_vcnt++;
}
EXPORT_SYMBOL(__bio_add_page);

int
bio_add_page(struct bio *bio, struct page *page,
             unsigned int len, unsigned int offset)
{
    if (bio_full(bio, len))
        return 0;
    __bio_add_page(bio, page, len, offset);
    return len;
}
EXPORT_SYMBOL(bio_add_page);

void bio_advance(struct bio *bio, unsigned bytes)
{
    bio_advance_iter(bio, &bio->bi_iter, bytes);
}
EXPORT_SYMBOL(bio_advance);

void bio_endio(struct bio *bio)
{
    if (bio->bi_end_io)
        bio->bi_end_io(bio);
}
EXPORT_SYMBOL(bio_endio);

static void biovec_init_slabs(void)
{
    int i;

    for (i = 0; i < BVEC_POOL_NR; i++) {
        int size;
        struct biovec_slab *bvs = bvec_slabs + i;

        if (bvs->nr_vecs <= BIO_INLINE_VECS) {
            bvs->slab = NULL;
            continue;
        }

        size = bvs->nr_vecs * sizeof(struct bio_vec);
        bvs->slab = kmem_cache_create(bvs->name, size, 0,
                                      SLAB_HWCACHE_ALIGN|SLAB_PANIC,
                                      NULL);
    }
}

static int init_bio(void)
{
    biovec_init_slabs();

    if (bioset_init(&fs_bio_set, BIO_POOL_SIZE, 0, BIOSET_NEED_BVECS))
        panic("bio: can't allocate bios\n");

    return 0;
}

int
init_module(void)
{
    printk("module[bio]: init begin ...\n");
    init_bio();
    printk("module[bio]: init end!\n");
    return 0;
}

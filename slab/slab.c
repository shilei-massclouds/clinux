// SPDX-License-Identifier: GPL-2.0-only
//#define X_DEBUG
#include <mm.h>
#include <errno.h>
#include <list.h>
#include <log2.h>
#include <slab.h>
#include <kernel.h>
#include <string.h>
#include <printk.h>

#include <export.h>

#define BYTES_PER_WORD  sizeof(void *)

#define CFLGS_OBJFREELIST_SLAB  ((slab_flags_t)0x40000000U)
#define OBJFREELIST_SLAB(x) ((x)->flags & CFLGS_OBJFREELIST_SLAB)

#define CACHE_CACHE 0
#define SIZE_NODE   1

#define ARCH_KMALLOC_FLAGS SLAB_HWCACHE_ALIGN

#define INDEX_NODE kmalloc_index(sizeof(struct kmem_cache_node))

#define SLAB_OBJ_MAX_NUM \
    ((1 << sizeof(freelist_idx_t) * BITS_PER_BYTE) - 1)

#define MAKE_LIST(cachep, listp, slab)          \
    do {                                        \
        INIT_LIST_HEAD(listp);                  \
        list_splice(&cachep->node->slab, listp); \
    } while (0)

typedef unsigned char freelist_idx_t;

struct array_cache {
    unsigned int avail;
    unsigned int limit;
    unsigned int batchcount;
    void *entry[];
    /*
     * Must have this definition in here for the proper
     * alignment of array_cache. Also simplifies accessing
     * the entries.
     */
};

extern size_t reserved_chunk_size;
extern void *reserved_chunk_ptr;

LIST_HEAD(slab_caches);

#define NUM_INIT_LISTS 2
static struct kmem_cache_node init_kmem_cache_node[NUM_INIT_LISTS];

enum slab_state slab_state;
struct kmem_cache *kmem_cache;

#define BOOT_CPUCACHE_ENTRIES   1
/* internal cache of cache description objs */
static struct kmem_cache kmem_cache_boot = {
    .limit = BOOT_CPUCACHE_ENTRIES,
    .size = sizeof(struct kmem_cache),
    .name = "kmem_cache",
};

/*
 * Conversion table for small slabs sizes / 8 to the index in the
 * kmalloc array. This is necessary for slabs < 192 since we have
 * non power of two cache sizes there. The size of larger slabs can
 * be determined using fls.
 */
static u8
size_index[24] = {
    3,  /* 8 */
    4,  /* 16 */
    5,  /* 24 */
    5,  /* 32 */
    6,  /* 40 */
    6,  /* 48 */
    6,  /* 56 */
    6,  /* 64 */
    1,  /* 72 */
    1,  /* 80 */
    1,  /* 88 */
    1,  /* 96 */
    7,  /* 104 */
    7,  /* 112 */
    7,  /* 120 */
    7,  /* 128 */
    2,  /* 136 */
    2,  /* 144 */
    2,  /* 152 */
    2,  /* 160 */
    2,  /* 168 */
    2,  /* 176 */
    2,  /* 184 */
    2   /* 192 */
};

#define INIT_KMALLOC_INFO(__size, __short_size) \
{                                               \
    .name = "kmalloc-" #__short_size,           \
    .size = __size,                             \
}

const struct kmalloc_info_struct kmalloc_info[] = {
    INIT_KMALLOC_INFO(0, 0),
    INIT_KMALLOC_INFO(96, 96),
    INIT_KMALLOC_INFO(192, 192),
    INIT_KMALLOC_INFO(8, 8),
    INIT_KMALLOC_INFO(16, 16),
    INIT_KMALLOC_INFO(32, 32),
    INIT_KMALLOC_INFO(64, 64),
    INIT_KMALLOC_INFO(128, 128),
    INIT_KMALLOC_INFO(256, 256),
    INIT_KMALLOC_INFO(512, 512),
    INIT_KMALLOC_INFO(1024, 1k),
    INIT_KMALLOC_INFO(2048, 2k),
    INIT_KMALLOC_INFO(4096, 4k),
    INIT_KMALLOC_INFO(8192, 8k),
    INIT_KMALLOC_INFO(16384, 16k),
    INIT_KMALLOC_INFO(32768, 32k),
    INIT_KMALLOC_INFO(65536, 64k),
    INIT_KMALLOC_INFO(131072, 128k),
    INIT_KMALLOC_INFO(262144, 256k),
    INIT_KMALLOC_INFO(524288, 512k),
    INIT_KMALLOC_INFO(1048576, 1M),
    INIT_KMALLOC_INFO(2097152, 2M),
    INIT_KMALLOC_INFO(4194304, 4M),
    INIT_KMALLOC_INFO(8388608, 8M),
    INIT_KMALLOC_INFO(16777216, 16M),
    INIT_KMALLOC_INFO(33554432, 32M),
    INIT_KMALLOC_INFO(67108864, 64M)
};

struct kmem_cache *
kmalloc_caches[KMALLOC_SHIFT_HIGH + 1] = {};

static inline unsigned int
size_index_elem(unsigned int bytes)
{
    return (bytes - 1) / 8;
}

struct kmem_cache *
kmalloc_slab(size_t size, gfp_t flags)
{
    unsigned int index;

    if (size <= 192) {
        if (!size)
            return ZERO_SIZE_PTR;

        index = size_index[size_index_elem(size)];
    } else {
        BUG_ON(size > KMALLOC_MAX_CACHE_SIZE);
        index = fls(size - 1);
    }

    return kmalloc_caches[index];
}

static struct page *
get_first_slab(struct kmem_cache_node *n)
{
    struct page *page;

    page = list_first_entry_or_null(&n->slabs_partial,
                                    struct page, slab_list);
    if (!page) {
        page = list_first_entry_or_null(&n->slabs_free,
                                        struct page, slab_list);
        if (page)
            n->free_slabs--;
    }

    return page;
}

static inline void *
index_to_obj(struct kmem_cache *cache,
             struct page *page,
             unsigned int idx)
{
    return page->s_mem + cache->size * idx;
}

static inline freelist_idx_t
get_free_obj(struct page *page, unsigned int idx)
{
    BUG_ON(page->freelist == NULL);
    return ((freelist_idx_t *)page->freelist)[idx];
}

static void *
slab_get_obj(struct kmem_cache *cachep, struct page *page)
{
    void *objp;
    objp = index_to_obj(cachep, page, get_free_obj(page, page->active));
    page->active++;
    return objp;
}

static __always_inline int
alloc_block(struct kmem_cache *cachep,
            struct array_cache *ac,
            struct page *page,
            int batchcount)
{
    /*
     * There must be at least one object available for
     * allocation.
     */
    BUG_ON(page->active >= cachep->num);
    while (page->active < cachep->num && batchcount--) {
        ac->entry[ac->avail++] = slab_get_obj(cachep, page);
    }
    return batchcount;
}

static inline void
fixup_slab_list(struct kmem_cache *cachep,
                struct kmem_cache_node *n, struct page *page,
                void **list)
{
    /* move slabp to correct slabp list: */
    list_del(&page->slab_list);
    if (page->active == cachep->num) {
        list_add(&page->slab_list, &n->slabs_full);
        if (OBJFREELIST_SLAB(cachep))
            page->freelist = NULL;
    } else {
        list_add(&page->slab_list, &n->slabs_partial);
    }
}

static inline gfp_t
gfp_exact_node(gfp_t flags)
{
    return flags & ~__GFP_NOFAIL;
}

/*
 * Interface to system's page allocator. No need to hold the
 * kmem_cache_node ->list_lock.
 *
 * If we requested dmaable memory, we will get it. Even if we
 * did not request dmaable memory, we might get it, but that
 * would be relatively rare and ignorable.
 */
static struct page *
kmem_getpages(struct kmem_cache *cachep, gfp_t flags)
{
    struct page *page;
    page = alloc_pages(flags, cachep->gfporder);
    if (!page) {
        panic("slab out of memory!");
        return NULL;
    }

    __SetPageSlab(page);
    return page;
}

static void *
alloc_slabmgmt(struct kmem_cache *cachep,
               struct page *page,
               gfp_t local_flags)
{
    void *freelist;
    void *addr = page_address(page);

    page->s_mem = addr;
    page->active = 0;

    if (OBJFREELIST_SLAB(cachep)) {
        freelist = NULL;
    } else {
        /* We will use last bytes at the slab for freelist */
        freelist = addr +
            (PAGE_SIZE << cachep->gfporder) - cachep->freelist_size;
    }

    return freelist;
}

static void
slab_map_pages(struct kmem_cache *cache,
               struct page *page,
               void *freelist)
{
    page->slab_cache = cache;
    page->freelist = freelist;
}

static inline void
set_free_obj(struct page *page, unsigned int idx, freelist_idx_t val)
{
    ((freelist_idx_t *)(page->freelist))[idx] = val;
}

static void
cache_init_objs(struct kmem_cache *cachep, struct page *page)
{
    int i;
    void *objp;

    if (OBJFREELIST_SLAB(cachep)) {
        page->freelist = index_to_obj(cachep, page, cachep->num - 1);
    }

    for (i = 0; i < cachep->num; i++) {
        objp = index_to_obj(cachep, page, i);
        if (cachep->ctor) {
            cachep->ctor(objp);
        }

        set_free_obj(page, i, i);
    }
}

static struct page *
cache_grow_begin(struct kmem_cache *cachep, gfp_t flags)
{
    void *freelist;
    struct page *page;

    /*
     * Get mem for the objs. Attempt to allocate a physical page.
     */
    page = kmem_getpages(cachep, flags);
    if (!page)
        goto failed;

    /* Get slab management. */
    freelist = alloc_slabmgmt(cachep, page, flags);
    slab_map_pages(cachep, page, freelist);
    cache_init_objs(cachep, page);
    return page;

 failed:
    panic("%s: ... (%u)", __func__, flags);
    return NULL;
}

static void
cache_grow_end(struct kmem_cache *cachep, struct page *page)
{
    struct kmem_cache_node *n;
    void *list = NULL;

    if (!page)
        return;

    INIT_LIST_HEAD(&page->slab_list);
    n = cachep->node;

    n->total_slabs++;
    if (!page->active) {
        list_add_tail(&page->slab_list, &n->slabs_free);
        n->free_slabs++;
    } else {
        fixup_slab_list(cachep, n, page, &list);
    }

    n->free_objects += cachep->num - page->active;
}

static void *
cache_alloc_refill(struct kmem_cache *cachep, gfp_t flags)
{
    int batchcount;
    struct array_cache *ac;
    struct kmem_cache_node *n;
    struct page *page;
    void *list = NULL;

    ac = cpu_cache_get(cachep);
    batchcount = ac->batchcount;
    n = cachep->node;

    BUG_ON(ac->avail > 0 || !n);
    if (!n->free_objects) {
        goto direct_grow;
    }

    while (batchcount > 0) {
        /* Get slab alloc is to come from. */
        page = get_first_slab(n);
        if (!page)
            goto must_grow;

        batchcount = alloc_block(cachep, ac, page, batchcount);
        fixup_slab_list(cachep, n, page, &list);
    }

 must_grow:
    n->free_objects -= ac->avail;

 direct_grow:
    if (unlikely(!ac->avail)) {
        page = cache_grow_begin(cachep, gfp_exact_node(flags));

        /*
         * cache_grow_begin() can reenable interrupts,
         * then ac could change.
         */
        ac = cpu_cache_get(cachep);
        if (!ac->avail && page)
            alloc_block(cachep, ac, page, batchcount);
        cache_grow_end(cachep, page);

        if (!ac->avail)
            return NULL;
    }

    return ac->entry[--ac->avail];
}

static inline void *
____cache_alloc(struct kmem_cache *cachep, gfp_t flags)
{
    struct array_cache *ac;
    ac = cpu_cache_get(cachep);
    if (likely(ac->avail)) {
        return ac->entry[--ac->avail];
    }
    return cache_alloc_refill(cachep, flags);
}

static __always_inline void *
__do_cache_alloc(struct kmem_cache *cachep, gfp_t flags)
{
    return ____cache_alloc(cachep, flags);
}

static inline bool
slab_want_init_on_alloc(gfp_t flags, struct kmem_cache *c)
{
    return flags & __GFP_ZERO;
}

static __always_inline void *
slab_alloc(struct kmem_cache *cachep, gfp_t flags, unsigned long caller)
{
    void *objp;

    objp = __do_cache_alloc(cachep, flags);
    if (unlikely(slab_want_init_on_alloc(flags, cachep)) && objp)
        memset(objp, 0, cachep->object_size);

    return objp;
}

static __always_inline void *
__do_kmalloc(size_t size, gfp_t flags, unsigned long caller)
{
    struct kmem_cache *cachep;
    void *ret;

    if (unlikely(size > KMALLOC_MAX_CACHE_SIZE))
        return NULL;

    cachep = kmalloc_slab(size, flags);
    if (unlikely(ZERO_OR_NULL_PTR(cachep)))
        return cachep;

    return slab_alloc(cachep, flags, caller);
}

void *
__kmalloc(size_t size, gfp_t flags)
{
    return __do_kmalloc(size, flags, _RET_IP_);
}

static void *
_kmalloc(size_t size, gfp_t flags)
{
    return __kmalloc(size, flags);
}

char *
_kmemdup_nul(const char *s, size_t len, gfp_t gfp)
{
    char *buf;

    if (!s)
        return NULL;

    buf = _kmalloc(len + 1, gfp);
    if (buf) {
        memcpy(buf, s, len);
        buf[len] = '\0';
    }
    return buf;
}

static void
kmem_cache_node_init(struct kmem_cache_node *node)
{
    INIT_LIST_HEAD(&node->slabs_full);
    INIT_LIST_HEAD(&node->slabs_partial);
    INIT_LIST_HEAD(&node->slabs_free);
    node->total_slabs = 0;
    node->free_slabs = 0;
    node->free_objects = 0;
}

static unsigned int
calculate_alignment(slab_flags_t flags,
                    unsigned int align, unsigned int size)
{
    /*
     * If the user wants hardware cache aligned objects then follow that
     * suggestion if the object is sufficiently large.
     *
     * The hardware cache alignment cannot override the specified
     * alignment though. If that is greater then use it.
     */
    if (flags & SLAB_HWCACHE_ALIGN) {
        unsigned int ralign;

        ralign = cache_line_size();
        while (size <= ralign / 2)
            ralign /= 2;
        align = max(align, ralign);
    }

    if (align < ARCH_SLAB_MINALIGN)
        align = ARCH_SLAB_MINALIGN;

    return ALIGN(align, sizeof(void *));
}

bool
slab_is_available(void)
{
    return slab_state >= UP;
}
EXPORT_SYMBOL(slab_is_available);

static void
init_arraycache(struct array_cache *ac, int limit, int batch)
{
    if (ac) {
        ac->avail = 0;
        ac->limit = limit;
        ac->batchcount = batch;
    }
}

static struct array_cache *
alloc_kmem_cache_cpus(struct kmem_cache *cachep,
                      int entries,
                      int batchcount)
{
    int cpu;
    size_t size;
    struct array_cache *cpu_cache;

    if (!reserved_chunk_ptr || !reserved_chunk_size)
        panic("no reserved memory for cpu_cache");

    size = sizeof(void *) * entries + sizeof(struct array_cache);
    if (size > reserved_chunk_size)
        panic("need %lu, but left reserved chunk is %lu",
              size, reserved_chunk_size);

    cpu_cache = reserved_chunk_ptr;

    reserved_chunk_ptr += size;
    reserved_chunk_size -= size;

    init_arraycache(cpu_cache, entries, batchcount);
    return cpu_cache;
}

static void
set_up_node(struct kmem_cache *cachep, int index)
{
    cachep->node = &init_kmem_cache_node[index];
}

static int
init_cache_node(struct kmem_cache *cachep, gfp_t gfp)
{
    struct kmem_cache_node *n;

    if (cachep->node)
        return 0;

    n = kmalloc(sizeof(struct kmem_cache_node), gfp);
    if (!n)
        return -ENOMEM;

    kmem_cache_node_init(n);
    cachep->node = n;
    return 0;
}

/*
 * Interface to system's page release.
 */
static void
kmem_freepages(struct kmem_cache *cachep, struct page *page)
{
    int order = cachep->gfporder;

    BUG_ON(!PageSlab(page));
    __ClearPageSlab(page);

    __free_pages(page, order);
}

static void
slab_destroy(struct kmem_cache *cachep, struct page *page)
{
    void *freelist;

    freelist = page->freelist;
    kmem_freepages(cachep, page);
}

static void
slabs_destroy(struct kmem_cache *cachep, struct list_head *list)
{
    struct page *page, *n;

    list_for_each_entry_safe(page, n, list, slab_list) {
        list_del(&page->slab_list);
        slab_destroy(cachep, page);
    }
}

static int
setup_kmem_cache_node(struct kmem_cache *cachep, gfp_t gfp, bool force_change)
{
    if (init_cache_node(cachep, gfp))
        panic("cannot init cache node!");

    return 0;
}

static int
setup_kmem_cache_nodes(struct kmem_cache *cachep, gfp_t gfp)
{
    int ret;
    ret = setup_kmem_cache_node(cachep, gfp, true);
    if (ret)
        panic("setup kmem_cache_node failed!");

    return 0;
}

static void
slab_put_obj(struct kmem_cache *cachep, struct page *page, void *objp)
{
    unsigned int objnr = obj_to_index(cachep, page, objp);
    page->active--;
    if (!page->freelist)
        page->freelist = objp;

    set_free_obj(page, page->active, objnr);
}

static void
free_block(struct kmem_cache *cachep,
           void **objpp,
           int nr_objects,
           struct list_head *list)
{
    int i;
    struct page *page;
    struct kmem_cache_node *n = cachep->node;

    n->free_objects += nr_objects;

    for (i = 0; i < nr_objects; i++) {
        void *objp;
        struct page *page;

        objp = objpp[i];

        page = virt_to_head_page(objp);
        list_del(&page->slab_list);
        slab_put_obj(cachep, page, objp);

        /* fixup slab chains */
        if (page->active == 0) {
            list_add(&page->slab_list, &n->slabs_free);
            n->free_slabs++;
        } else {
            /* Unconditionally move a slab to the end of the
             * partial list on free - maximum time for the
             * other objects to be freed, too.
             */
            list_add_tail(&page->slab_list, &n->slabs_partial);
        }
    }
}

static int
do_tune_cpucache(struct kmem_cache *cachep,
                 int limit,
                 int batchcount,
                 gfp_t gfp)
{
    struct array_cache *cpu_cache;
    struct array_cache *prev;
    LIST_HEAD(list);

    cpu_cache = alloc_kmem_cache_cpus(cachep, limit, batchcount);
    if (!cpu_cache)
        return -ENOMEM;

    prev = cachep->cpu_cache;
    cachep->cpu_cache = cpu_cache;

    cachep->limit = limit;

    if (!prev)
        goto setup_node;

    free_block(cachep, prev->entry, prev->avail, &list);
    slabs_destroy(cachep, &list);

 setup_node:
    return setup_kmem_cache_nodes(cachep, gfp);
}

static int
enable_cpucache(struct kmem_cache *cachep, gfp_t gfp)
{
    int err;
    int limit = 0;
    int batchcount = 0;

    if (cachep->size > 131072)
        limit = 1;
    else if (cachep->size > PAGE_SIZE)
        limit = 8;
    else if (cachep->size > 1024)
        limit = 24;
    else if (cachep->size > 256)
        limit = 54;
    else
        limit = 120;

    batchcount = (limit + 1) / 2;
    err = do_tune_cpucache(cachep, limit, batchcount, gfp);
    if (err)
        panic("enable_cpucache failed for %s, error %d", cachep->name, -err);

    return err;
}

static int
setup_cpu_cache(struct kmem_cache *cachep, gfp_t gfp)
{
    if (slab_state >= FULL)
        return enable_cpucache(cachep, gfp);

    cachep->cpu_cache = alloc_kmem_cache_cpus(cachep, 1, 1);
    if (!cachep->cpu_cache)
        return 1;

    if (slab_state == DOWN) {
        /* Creation of first cache (kmem_cache). */
        set_up_node(kmem_cache, CACHE_CACHE);
    } else if (slab_state == PARTIAL) {
        /* For kmem_cache_node */
        set_up_node(cachep, SIZE_NODE);
    } else {
        cachep->node = kmalloc(sizeof(struct kmem_cache_node), gfp);
        BUG_ON(!cachep->node);
        kmem_cache_node_init(cachep->node);
    }

    cpu_cache_get(cachep)->avail = 0;
    cpu_cache_get(cachep)->limit = BOOT_CPUCACHE_ENTRIES;
    cpu_cache_get(cachep)->batchcount = 1;
    cachep->limit = BOOT_CPUCACHE_ENTRIES;
    return 0;
}

static unsigned int
cache_estimate(unsigned long gfporder,
               size_t buffer_size,
               slab_flags_t flags,
               size_t *left_over)
{
    unsigned int num;
    size_t slab_size = PAGE_SIZE << gfporder;

    if (flags & (CFLGS_OBJFREELIST_SLAB)) {
        num = slab_size / buffer_size;
        *left_over = slab_size % buffer_size;
    } else {
        num = slab_size / (buffer_size + sizeof(freelist_idx_t));
        *left_over = slab_size %
            (buffer_size + sizeof(freelist_idx_t));
    }
    return num;
}

static size_t
calculate_slab_order(struct kmem_cache *cachep,
                     size_t size,
                     slab_flags_t flags)
{
    size_t left_over = 0;
    int gfporder;

    for (gfporder = 0; gfporder <= KMALLOC_MAX_ORDER; gfporder++) {
        unsigned int num;
        size_t remainder;

        num = cache_estimate(gfporder, size, flags, &remainder);
        if (!num)
            continue;

        /* Can't handle number of objects more than SLAB_OBJ_MAX_NUM */
        if (num > SLAB_OBJ_MAX_NUM)
            break;

        /* Found something acceptable - save it away */
        cachep->num = num;
        cachep->gfporder = gfporder;
        left_over = remainder;
        break;
    }
    return left_over;
}

static bool
set_objfreelist_slab_cache(struct kmem_cache *cachep,
                           size_t size,
                           slab_flags_t flags)
{
    cachep->num = 0;

    if (cachep->ctor)
        return false;

    calculate_slab_order(cachep, size, flags | CFLGS_OBJFREELIST_SLAB);
    if (!cachep->num)
        return false;

    if (cachep->num * sizeof(freelist_idx_t) > cachep->object_size)
        return false;

    return true;
}

static bool
set_on_slab_cache(struct kmem_cache *cachep, size_t size, slab_flags_t flags)
{
    size_t left;

    cachep->num = 0;

    left = calculate_slab_order(cachep, size, flags);
    if (!cachep->num)
        return false;

    return true;
}

int
__kmem_cache_create(struct kmem_cache *cachep, slab_flags_t flags)
{
    int err;
    gfp_t gfp;
    size_t ralign = BYTES_PER_WORD;
    unsigned int size = cachep->size;

    /*
     * Check that size is in terms of words.  This is needed to avoid
     * unaligned accesses for some archs when redzoning is used, and makes
     * sure any on-slab bufctl's are also correctly aligned.
     */
    size = ALIGN(size, BYTES_PER_WORD);

    if (ralign < cachep->align) {
        ralign = cachep->align;
    }

    cachep->align = ralign;

    if (slab_is_available())
        gfp = GFP_KERNEL;
    else
        gfp = GFP_NOWAIT;

    size = ALIGN(size, cachep->align);

    if (size < SLAB_OBJ_MIN_SIZE)
        size = ALIGN(SLAB_OBJ_MIN_SIZE, cachep->align);

    if (set_objfreelist_slab_cache(cachep, size, flags)) {
        flags |= CFLGS_OBJFREELIST_SLAB;
        goto done;
    }

    if (set_on_slab_cache(cachep, size, flags))
        goto done;

    panic("bad cache!");
    return -E2BIG;

 done:
    cachep->freelist_size = cachep->num * sizeof(freelist_idx_t);
    cachep->flags = flags;
    cachep->size = size;

    err = setup_cpu_cache(cachep, gfp);
    if (err) {
        panic("cannot create cpu cache!");
    }
    return 0;
}

/* Create a cache during boot when no slab services are available yet */
void
create_boot_cache(struct kmem_cache *s, const char *name,
                  unsigned int size, slab_flags_t flags)
{
    int err;
    unsigned int align = ARCH_KMALLOC_MINALIGN;

    s->name = name;
    s->size = s->object_size = size;

    /*
     * For power of two sizes, guarantee natural alignment for kmalloc
     * caches, regardless of SL*B debugging options.
     */
    if (is_power_of_2(size))
        align = max(align, size);
    s->align = calculate_alignment(flags, align, size);

    err = __kmem_cache_create(s, flags);
    if (err)
        panic("Creation of kmalloc slab %s size=%u failed. Reason %d",
              name, size, err);
}

struct kmem_cache *
create_kmalloc_cache(const char *name, unsigned int size, slab_flags_t flags)
{
    struct kmem_cache *s = kmem_cache_zalloc(kmem_cache, GFP_NOWAIT);
    if (!s)
        panic("Out of memory when creating slab %s\n", name);

    create_boot_cache(s, name, size, flags);
    list_add(&s->list, &slab_caches);
    return s;
}

static void *
_kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags)
{
    return slab_alloc(cachep, flags, _RET_IP_);
}

void setup_kmalloc_cache_index_table(void)
{
    unsigned int i;

    for (i = 8; i < KMALLOC_MIN_SIZE; i += 8) {
        unsigned int elem = size_index_elem(i);
        if (elem >= ARRAY_SIZE(size_index))
            break;
        size_index[elem] = KMALLOC_SHIFT_LOW;
    }
}

/*
 * Initialisation.
 * Called after the page allocator have been initialised and
 * before smp_init().
 */
static void
init_list(struct kmem_cache *cachep, struct kmem_cache_node *list)
{
    struct kmem_cache_node *ptr;

    ptr = kmalloc(sizeof(struct kmem_cache_node), GFP_NOWAIT);
    BUG_ON(!ptr);

    memcpy(ptr, list, sizeof(struct kmem_cache_node));

    MAKE_LIST((cachep), (&(ptr)->slabs_full), slabs_full);
    MAKE_LIST((cachep), (&(ptr)->slabs_free), slabs_free);
    MAKE_LIST((cachep), (&(ptr)->slabs_partial), slabs_partial);

    cachep->node = ptr;
}

static void
new_kmalloc_cache(int idx, slab_flags_t flags)
{
    kmalloc_caches[idx] =
        create_kmalloc_cache(kmalloc_info[idx].name,
                             kmalloc_info[idx].size,
                             flags);
}

void
create_kmalloc_caches(slab_flags_t flags)
{
    int i;

    for (i = KMALLOC_SHIFT_LOW; i <= KMALLOC_SHIFT_HIGH; i++) {
        if (!kmalloc_caches[i])
            new_kmalloc_cache(i, flags);

        /*
         * Caches that are not of the two-to-the-power-of size.
         * These have to be created immediately after the
         * earlier power of two caches
         */
        if (i == 6 && !kmalloc_caches[1])
            new_kmalloc_cache(1, flags);
        if (i == 7 && !kmalloc_caches[2])
            new_kmalloc_cache(2, flags);
    }

    /* Kmalloc array is now usable */
    slab_state = UP;
}

void
kmem_cache_init(void)
{
    int i;

    /* Static cache_cache */
    kmem_cache = &kmem_cache_boot;

    /* Static node_cache for cache_cache and node_cache */
    for (i = 0; i < NUM_INIT_LISTS; i++)
        kmem_cache_node_init(&init_kmem_cache_node[i]);

    /* Init cache_cache and attach static node_cache for it. */
    create_boot_cache(kmem_cache, "kmem_cache",
                      sizeof(struct kmem_cache),
                      SLAB_HWCACHE_ALIGN);
    list_add(&kmem_cache->list, &slab_caches);
    slab_state = PARTIAL;

    /* Create node_cache by cache_cache */
    kmalloc_caches[INDEX_NODE] =
        create_kmalloc_cache(kmalloc_info[INDEX_NODE].name,
                             kmalloc_info[INDEX_NODE].size,
                             ARCH_KMALLOC_FLAGS);
    slab_state = PARTIAL_NODE;

    /* Adjust index table */
    setup_kmalloc_cache_index_table();

    init_list(kmem_cache, &init_kmem_cache_node[CACHE_CACHE]);

    init_list(kmalloc_caches[INDEX_NODE], &init_kmem_cache_node[SIZE_NODE]);

    create_kmalloc_caches(ARCH_KMALLOC_FLAGS);
}

static void
cache_flusharray(struct kmem_cache *cachep, struct array_cache *ac)
{
    int batchcount;
    LIST_HEAD(list);

    batchcount = ac->batchcount;

    free_block(cachep, ac->entry, batchcount, &list);

free_done:
    slabs_destroy(cachep, &list);
    ac->avail -= batchcount;
    memmove(ac->entry, &(ac->entry[batchcount]), sizeof(void *)*ac->avail);
}

static __always_inline void
__free_one(struct array_cache *ac, void *objp)
{
    ac->entry[ac->avail++] = objp;
}

void
___cache_free(struct kmem_cache *cachep, void *objp, unsigned long caller)
{
    struct array_cache *ac = cpu_cache_get(cachep);

    if (ac->avail >= ac->limit)
        cache_flusharray(cachep, ac);

    __free_one(ac, objp);
}

static __always_inline void
__cache_free(struct kmem_cache *cachep, void *objp, unsigned long caller)
{
    ___cache_free(cachep, objp, caller);
}

/**
 * kfree - free previously allocated memory
 * @objp: pointer returned by kmalloc.
 *
 * If @objp is NULL, no operation is performed.
 *
 * Don't free memory not originally allocated by kmalloc()
 * or you will run into trouble.
 */
static void
_kfree(const void *objp)
{
    struct kmem_cache *c;
    unsigned long flags;

    if (unlikely(ZERO_OR_NULL_PTR(objp)))
        return;
    c = virt_to_cache(objp);
    if (!c) {
        return;
    }
    __cache_free(c, (void *)objp, _RET_IP_);
}

void
kmem_cache_init_late(void)
{
    struct kmem_cache *cachep;

    /* 6) resize the head arrays to their final sizes */
    list_for_each_entry(cachep, &slab_caches, list)
        if (enable_cpucache(cachep, GFP_NOWAIT))
            BUG();

    /* Done! */
    slab_state = FULL;
}

static struct kmem_cache *
create_cache(const char *name,
             unsigned int object_size,
             unsigned int align,
             slab_flags_t flags,
             unsigned int useroffset,
             unsigned int usersize,
             void (*ctor)(void *),
             struct kmem_cache *root_cache)
{
    struct kmem_cache *s;
    int err;

    BUG_ON(useroffset + usersize > object_size);

    err = -ENOMEM;
    s = kmem_cache_zalloc(kmem_cache, GFP_KERNEL);
    if (!s)
        goto out;

    s->name = name;
    s->size = s->object_size = object_size;
    s->align = align;
    s->ctor = ctor;

    err = __kmem_cache_create(s, flags);
    if (err)
        goto out;

    list_add(&s->list, &slab_caches);
out:
    if (err)
        return ERR_PTR(err);
    return s;
}

struct kmem_cache *
kmem_cache_create_usercopy(const char *name,
                           unsigned int size,
                           unsigned int align,
                           slab_flags_t flags,
                           unsigned int useroffset,
                           unsigned int usersize,
                           void (*ctor)(void *))
{
    const char *cache_name;
    int err = 0;
    struct kmem_cache *s = NULL;

    cache_name = kstrdup_const(name, GFP_KERNEL);
    if (!cache_name) {
        err = -ENOMEM;
        goto out_unlock;
    }

    s = create_cache(cache_name, size,
                     calculate_alignment(flags, align, size),
                     flags, useroffset, usersize, ctor, NULL);
    if (IS_ERR(s)) {
        err = PTR_ERR(s);
        kfree_const(cache_name);
    }

out_unlock:

    if (err) {
        if (flags & SLAB_PANIC) {
            panic("kmem_cache_create: Failed to create slab '%s'.(%d)",
                  name, err);
        } else {
            panic("kmem_cache_create(%s) failed with error %d",
                  name, err);
        }
        return NULL;
    }
    return s;
}
EXPORT_SYMBOL(kmem_cache_create_usercopy);

struct kmem_cache *
kmem_cache_create(const char *name,
                  unsigned int size,
                  unsigned int align,
                  slab_flags_t flags,
                  void (*ctor)(void *))
{
    return kmem_cache_create_usercopy(name, size, align, flags, 0, 0, ctor);
}
EXPORT_SYMBOL(kmem_cache_create);

static void
_kmem_cache_free(struct kmem_cache *cachep, void *objp)
{
    unsigned long flags;
    cachep = cache_from_obj(cachep, objp);
    if (!cachep)
        return;

    __cache_free(cachep, objp, _RET_IP_);
}

int
init_module(void)
{
    printk("module[slab]: init begin ...\n");

    kmalloc = _kmalloc;
    kfree = _kfree;

    kmemdup_nul = _kmemdup_nul;

    kmem_cache_alloc = _kmem_cache_alloc;
    kmem_cache_free = _kmem_cache_free;

    kmem_cache_init();
    kmem_cache_init_late();

    printk("module[slab]: init end!\n");
    return 0;
}

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MEMPOOL_H
#define _LINUX_MEMPOOL_H

typedef void * (mempool_alloc_t)(gfp_t gfp_mask, void *pool_data);
typedef void (mempool_free_t)(void *element, void *pool_data);

typedef struct mempool_s {
    int min_nr;         /* nr of elements at *elements */
    int curr_nr;        /* Current nr of elements at *elements */
    void **elements;

    void *pool_data;
    mempool_alloc_t *alloc;
    mempool_free_t *free;
} mempool_t;

int mempool_init(mempool_t *pool, int min_nr, mempool_alloc_t *alloc_fn,
                 mempool_free_t *free_fn, void *pool_data);

/*
 * A mempool_alloc_t and mempool_free_t that get the memory from
 * a slab cache that is passed in through pool_data.
 * Note: the slab cache may not have a ctor function.
 */
void *mempool_alloc_slab(gfp_t gfp_mask, void *pool_data);
void mempool_free_slab(void *element, void *pool_data);

static inline int
mempool_init_slab_pool(mempool_t *pool, int min_nr,
                       struct kmem_cache *kc)
{
    return mempool_init(pool, min_nr, mempool_alloc_slab,
                        mempool_free_slab, (void *) kc);
}

#endif /* _LINUX_MEMPOOL_H */

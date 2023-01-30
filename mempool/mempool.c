// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <export.h>
#include <printk.h>
#include <mempool.h>

/*
 * A commonly used alloc and free fn.
 */
void *mempool_alloc_slab(gfp_t gfp_mask, void *pool_data)
{
    struct kmem_cache *mem = pool_data;
    BUG_ON(mem->ctor);
    return kmem_cache_alloc(mem, gfp_mask);
}
EXPORT_SYMBOL(mempool_alloc_slab);

void mempool_free_slab(void *element, void *pool_data)
{
    struct kmem_cache *mem = pool_data;
    kmem_cache_free(mem, element);
}
EXPORT_SYMBOL(mempool_free_slab);

static __always_inline void add_element(mempool_t *pool, void *element)
{
    BUG_ON(pool->curr_nr >= pool->min_nr);
    pool->elements[pool->curr_nr++] = element;
}

int mempool_init_node(mempool_t *pool, int min_nr, mempool_alloc_t *alloc_fn,
                      mempool_free_t *free_fn, void *pool_data, gfp_t gfp_mask)
{
    pool->min_nr = min_nr;
    pool->pool_data = pool_data;
    pool->alloc = alloc_fn;
    pool->free  = free_fn;
    //init_waitqueue_head(&pool->wait);

    pool->elements = kmalloc_array(min_nr, sizeof(void *), gfp_mask);
    if (!pool->elements)
        panic("out of memory!");

    /*
     * First pre-allocate the guaranteed number of buffers.
     */
    while (pool->curr_nr < pool->min_nr) {
        void *element;

        element = pool->alloc(gfp_mask, pool->pool_data);
        if (unlikely(!element))
            panic("bad memory!");
        add_element(pool, element);
    }

    return 0;
}
EXPORT_SYMBOL(mempool_init_node);

/**
 * mempool_init - initialize a memory pool
 * @pool:      pointer to the memory pool that should be initialized
 * @min_nr:    the minimum number of elements guaranteed to be
 *             allocated for this pool.
 * @alloc_fn:  user-defined element-allocation function.
 * @free_fn:   user-defined element-freeing function.
 * @pool_data: optional private data available to the user-defined functions.
 *
 * Like mempool_create(), but initializes the pool in (i.e. embedded in another
 * structure).
 *
 * Return: %0 on success, negative error code otherwise.
 */
int mempool_init(mempool_t *pool, int min_nr, mempool_alloc_t *alloc_fn,
                 mempool_free_t *free_fn, void *pool_data)
{
    return mempool_init_node(pool, min_nr, alloc_fn, free_fn,
                             pool_data, GFP_KERNEL);
}
EXPORT_SYMBOL(mempool_init);

static int
init_module(void)
{
    printk("module[mempool]: init begin ...\n");

    printk("module[mempool]: init end!\n");
    return 0;
}

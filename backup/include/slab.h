/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SLAB_H
#define _LINUX_SLAB_H

#include <mm.h>
#include <bug.h>
#include <gfp.h>
#include <types.h>
#include <page.h>
#include <mmzone.h>
#include <kernel.h>
#include <compiler_attributes.h>

/* Panic if kmem_cache_create() fails */
# define SLAB_ACCOUNT   0

#define SLAB_RECLAIM_ACCOUNT    ((slab_flags_t __force)0x00020000U)
#define SLAB_PANIC              ((slab_flags_t __force)0x00040000U)
#define SLAB_MEM_SPREAD         ((slab_flags_t __force)0x00100000U)

#define KMALLOC_SHIFT_HIGH  (MAX_ORDER + PAGE_SHIFT - 1)
#define KMALLOC_SHIFT_MAX   KMALLOC_SHIFT_HIGH
#define KMALLOC_SHIFT_LOW   5

/* Maximum allocatable size */
#define KMALLOC_MAX_SIZE        (1UL << KMALLOC_SHIFT_MAX)
/* Maximum size for which we actually use a slab cache */
#define KMALLOC_MAX_CACHE_SIZE  (1UL << KMALLOC_SHIFT_HIGH)
/* Maximum order allocatable via the slab allocagtor */
#define KMALLOC_MAX_ORDER       (KMALLOC_SHIFT_MAX - PAGE_SHIFT)

/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

#define ZERO_OR_NULL_PTR(x) ((unsigned long)(x) <= (unsigned long)ZERO_SIZE_PTR)

#define KMALLOC_MIN_SIZE (1 << KMALLOC_SHIFT_LOW)

#define SLAB_OBJ_MIN_SIZE 16

#define SLAB_HWCACHE_ALIGN      ((slab_flags_t)0x00002000U)
#define ARCH_SLAB_MINALIGN      __alignof__(unsigned long long)
#define ARCH_KMALLOC_MINALIGN   __alignof__(unsigned long long)

/*
 * Please use this macro to create slab caches. Simply specify the
 * name of the structure and maybe some flags that are listed above.
 *
 * The alignment of the struct determines object alignment. If you
 * f.e. add ____cacheline_aligned_in_smp to the struct declaration
 * then the objects will be properly aligned in SMP configurations.
 */
#define KMEM_CACHE(__struct, __flags) \
    kmem_cache_create(#__struct, sizeof(struct __struct), \
                      __alignof__(struct __struct), (__flags), NULL)

struct kmem_cache *
kmem_cache_create_usercopy(const char *name,
                           unsigned int size,
                           unsigned int align,
                           slab_flags_t flags,
                           unsigned int useroffset,
                           unsigned int usersize,
                           void (*ctor)(void *));

/*
 * To whitelist a single field for copying to/from usercopy, use this
 * macro instead for KMEM_CACHE() above.
 */
#define KMEM_CACHE_USERCOPY(__struct, __flags, __field) \
    kmem_cache_create_usercopy(#__struct, \
                               sizeof(struct __struct), \
                               __alignof__(struct __struct), \
                               (__flags), \
                               offsetof(struct __struct, __field), \
                               sizeof_field(struct __struct, __field), \
                               NULL)

struct kmem_cache *
kmem_cache_create(const char *name, unsigned int size,
                  unsigned int align, slab_flags_t flags,
                  void (*ctor)(void *));

enum slab_state {
    DOWN,           /* No slab functionality yet */
    PARTIAL,        /* SLUB: kmem_cache_node available */
    PARTIAL_NODE,   /* SLAB: kmalloc size for node struct available */
    UP,             /* Slab caches usable but not all extras yet */
    FULL            /* Everything is working */
};

struct kmem_cache_node {
    struct list_head slabs_partial; /* partial list first, better asm code */
    struct list_head slabs_full;
    struct list_head slabs_free;
    unsigned long total_slabs;      /* length of all slab lists */
    unsigned long free_slabs;       /* length of free slab list only */
    unsigned long free_objects;
};

struct kmem_cache {
    struct array_cache *cpu_cache;

/* 1) Cache tunables. */
    unsigned int limit;
    unsigned int size;

/* 2) touched by every alloc & free from the backend */
    slab_flags_t flags; /* constant flags */
    unsigned int num;   /* # of objs per slab */

/* 3) cache_grow/shrink */
    /* order of pgs per slab (2^n) */
    unsigned int gfporder;
    unsigned int freelist_size;

    void (*ctor)(void *obj);

/* 4) cache creation/removal */
    const char *name;
    struct list_head list;
    int object_size;
    int align;

    struct kmem_cache_node *node;
};

extern const struct kmalloc_info_struct {
    const char *name;
    unsigned int size;
} kmalloc_info[];

extern struct kmem_cache *kmalloc_caches[];

void kmem_cache_init(void);

typedef void *
(*kmalloc_t)(size_t size, gfp_t flags);
extern kmalloc_t kmalloc;

typedef void *
(*krealloc_t)(const void *p, size_t new_size, gfp_t flags);
extern krealloc_t krealloc;

typedef void
(*kfree_t)(const void *objp);
extern kfree_t kfree;

typedef char *
(*kmemdup_nul_t)(const char *s, size_t len, gfp_t gfp);
extern kmemdup_nul_t kmemdup_nul;

static inline void *
kzalloc(size_t size, gfp_t flags)
{
    return kmalloc(size, flags | __GFP_ZERO);
}

static __always_inline void *
__kmalloc_node(size_t size, gfp_t flags)
{
    return kmalloc(size, flags);
}

static __always_inline void *
kmalloc_node(size_t size, gfp_t flags)
{
    return __kmalloc_node(size, flags);
}

static inline void *
kzalloc_node(size_t size, gfp_t flags)
{
    return kmalloc_node(size, flags | __GFP_ZERO);
}

/**
 * kmalloc_array - allocate memory for an array.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *
kmalloc_array(size_t n, size_t size, gfp_t flags)
{
    return kmalloc(n * size, flags);
}

/**
 * kcalloc - allocate memory for an array. The memory is set to zero.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *
kcalloc(size_t n, size_t size, gfp_t flags)
{
    return kmalloc_array(n, size, flags | __GFP_ZERO);
}

static __always_inline unsigned int
kmalloc_index(size_t size)
{
    if (!size)
        return 0;

    if (size <= KMALLOC_MIN_SIZE)
        return KMALLOC_SHIFT_LOW;

    if (size > 64 && size <= 96)
        return 1;
    if (size > 128 && size <= 192)
        return 2;

    if (size <=          8) return 3;
    if (size <=         16) return 4;
    if (size <=         32) return 5;
    if (size <=         64) return 6;
    if (size <=        128) return 7;
    if (size <=        256) return 8;
    if (size <=        512) return 9;
    if (size <=       1024) return 10;
    if (size <=   2 * 1024) return 11;
    if (size <=   4 * 1024) return 12;
    if (size <=   8 * 1024) return 13;
    if (size <=  16 * 1024) return 14;
    if (size <=  32 * 1024) return 15;
    if (size <=  64 * 1024) return 16;
    if (size <= 128 * 1024) return 17;
    if (size <= 256 * 1024) return 18;
    if (size <= 512 * 1024) return 19;
    if (size <= 1024 * 1024) return 20;
    if (size <=  2 * 1024 * 1024) return 21;
    if (size <=  4 * 1024 * 1024) return 22;
    if (size <=  8 * 1024 * 1024) return 23;
    if (size <=  16 * 1024 * 1024) return 24;
    if (size <=  32 * 1024 * 1024) return 25;
    if (size <=  64 * 1024 * 1024) return 26;
    BUG();

    /* Will never be reached. Needed because the compiler may complain */
    return -1;
}

typedef void *
(*kmem_cache_alloc_t)(struct kmem_cache *cachep, gfp_t flags);
extern kmem_cache_alloc_t kmem_cache_alloc;

typedef void
(*kmem_cache_free_t)(struct kmem_cache *cachep, void *objp);
extern kmem_cache_free_t kmem_cache_free;

static inline void *
kmem_cache_zalloc(struct kmem_cache *k, gfp_t flags)
{
    return kmem_cache_alloc(k, flags | __GFP_ZERO);
}

static inline struct array_cache *
cpu_cache_get(struct kmem_cache *cachep)
{
    return cachep->cpu_cache;
}

static inline struct kmem_cache *
virt_to_cache(const void *obj)
{
    struct page *page;

    page = virt_to_head_page(obj);
    if (page == NULL || !PageSlab(page))
        panic("Object is not a Slab page!");

    return page->slab_cache;
}

static inline unsigned int
obj_to_index(const struct kmem_cache *cache,
             const struct page *page,
             void *obj)
{
    return (obj - page->s_mem) / cache->size;
}

static inline struct kmem_cache *
cache_from_obj(struct kmem_cache *s, void *x)
{
    return s;
}

bool slab_is_available(void);

#endif /* _LINUX_SLAB_H */

// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <uio.h>
#include <export.h>
#include <kernel.h>
#include <printk.h>
#include <highmem.h>
#include <uaccess.h>

#define iterate_kvec(i, n, __v, __p, skip, STEP) {  \
    size_t wanted = n;              \
    __p = i->kvec;                  \
    __v.iov_len = min(n, __p->iov_len - skip);  \
    if (likely(__v.iov_len)) {          \
        __v.iov_base = __p->iov_base + skip;    \
        (void)(STEP);               \
        skip += __v.iov_len;            \
        n -= __v.iov_len;           \
    }                       \
    while (unlikely(n)) {               \
        __p++;                  \
        __v.iov_len = min(n, __p->iov_len); \
        if (unlikely(!__v.iov_len))     \
            continue;           \
        __v.iov_base = __p->iov_base;       \
        (void)(STEP);               \
        skip = __v.iov_len;         \
        n -= __v.iov_len;           \
    }                       \
    n = wanted;                 \
}

#define iterate_and_advance(i, n, v, I, B, K) {         \
    if (unlikely(i->count < n))             \
        n = i->count;                   \
    if (i->count) {                     \
        size_t skip = i->iov_offset;            \
        if (unlikely(i->type & ITER_BVEC)) {        \
            panic("no ITER_BVEC!");                 \
        } else if (unlikely(i->type & ITER_KVEC)) { \
            const struct kvec *kvec;        \
            struct kvec v;              \
            iterate_kvec(i, n, v, kvec, skip, (K))  \
            if (skip == kvec->iov_len) {        \
                kvec++;             \
                skip = 0;           \
            }                   \
            i->nr_segs -= kvec - i->kvec;       \
            i->kvec = kvec;             \
        } else if (unlikely(i->type & ITER_DISCARD)) {  \
            panic("no ITER_DISCARD!");      \
        } else {                    \
            panic("bad i->type!");      \
        }                       \
        i->count -= n;                  \
        i->iov_offset = skip;               \
    }                           \
}

void iov_iter_init(struct iov_iter *i, unsigned int direction,
                   const struct iovec *iov, unsigned long nr_segs,
                   size_t count)
{
    BUG_ON(direction & ~(READ | WRITE));
    direction &= READ | WRITE;

    /* It will get better.  Eventually... */
    if (uaccess_kernel()) {
        i->type = ITER_KVEC | direction;
        i->kvec = (struct kvec *)iov;
    } else {
        i->type = ITER_IOVEC | direction;
        i->iov = iov;
    }
    i->nr_segs = nr_segs;
    i->iov_offset = 0;
    i->count = count;
}
EXPORT_SYMBOL(iov_iter_init);

static inline bool page_copy_sane(struct page *page, size_t offset, size_t n)
{
    size_t v = n + offset;

    /*
     * The general case needs to access the page order in order
     * to compute the page size.
     * However, we mostly deal with order-0 pages and thus can
     * avoid a possible cache line miss for requests that fit all
     * page orders.
     */
    if (n <= v && v <= PAGE_SIZE)
        return true;

    panic("%s: !", __func__);
}

size_t copy_page_to_iter(struct page *page, size_t offset, size_t bytes,
                         struct iov_iter *i)
{
    if (unlikely(!page_copy_sane(page, offset, bytes)))
        return 0;

    if (i->type & (ITER_BVEC|ITER_KVEC)) {
        void *kaddr = kmap_atomic(page);
        size_t wanted = copy_to_iter(kaddr + offset, bytes, i);
        kunmap_atomic(kaddr);
        return wanted;
    }

    panic("%s: !", __func__);
}
EXPORT_SYMBOL(copy_page_to_iter);

static int copyout(void *to, const void *from, size_t n)
{
    panic("%s: !", __func__);
}

static void memcpy_to_page(struct page *page, size_t offset, const char *from, size_t len)
{
    char *to = kmap_atomic(page);
    memcpy(to + offset, from, len);
    kunmap_atomic(to);
}

size_t _copy_to_iter(const void *addr, size_t bytes, struct iov_iter *i)
{
    const char *from = addr;
    if (unlikely(iov_iter_is_pipe(i)))
        panic("is pipe!");
    if (iter_is_iovec(i))
        panic("is iovec!");

    iterate_and_advance(i, bytes, v,
        copyout(v.iov_base, (from += v.iov_len) - v.iov_len, v.iov_len),
        memcpy_to_page(v.bv_page, v.bv_offset,
                       (from += v.bv_len) - v.bv_len, v.bv_len),
        memcpy(v.iov_base, (from += v.iov_len) - v.iov_len, v.iov_len)
    )

    return bytes;
}
EXPORT_SYMBOL(_copy_to_iter);

int
init_module(void)
{
    printk("module[iov_iter]: init begin ...\n");
    printk("module[iov_iter]: init end!\n");
    return 0;
}

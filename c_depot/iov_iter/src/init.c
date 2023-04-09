// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <uio.h>
#include <export.h>
#include <kernel.h>
#include <printk.h>
#include <highmem.h>
#include <uaccess.h>
#include <slab.h>

#define iterate_iovec(i, n, __v, __p, skip, STEP) { \
    size_t left;                    \
    size_t wanted = n;              \
    __p = i->iov;                   \
    __v.iov_len = min(n, __p->iov_len - skip);  \
    if (likely(__v.iov_len)) {          \
        __v.iov_base = __p->iov_base + skip;    \
        left = (STEP);              \
        __v.iov_len -= left;            \
        skip += __v.iov_len;            \
        n -= __v.iov_len;           \
    } else {                    \
        left = 0;               \
    }                       \
    while (unlikely(!left && n)) {          \
        __p++;                  \
        __v.iov_len = min(n, __p->iov_len); \
        if (unlikely(!__v.iov_len))     \
            continue;           \
        __v.iov_base = __p->iov_base;       \
        left = (STEP);              \
        __v.iov_len -= left;            \
        skip = __v.iov_len;         \
        n -= __v.iov_len;           \
    }                       \
    n = wanted - n;                 \
}

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
            const struct iovec *iov;        \
            struct iovec v;             \
            iterate_iovec(i, n, v, iov, skip, (I))  \
            if (skip == iov->iov_len) {     \
                iov++;              \
                skip = 0;           \
            }                   \
            i->nr_segs -= iov - i->iov;     \
            i->iov = iov;               \
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

/**
 * rw_copy_check_uvector() - Copy an array of &struct iovec from userspace
 *     into the kernel and check that it is valid.
 *
 * @type: One of %CHECK_IOVEC_ONLY, %READ, or %WRITE.
 * @uvector: Pointer to the userspace array.
 * @nr_segs: Number of elements in userspace array.
 * @fast_segs: Number of elements in @fast_pointer.
 * @fast_pointer: Pointer to (usually small on-stack) kernel array.
 * @ret_pointer: (output parameter) Pointer to a variable that will point to
 *     either @fast_pointer, a newly allocated kernel array, or NULL,
 *     depending on which array was used.
 *
 * This function copies an array of &struct iovec of @nr_segs from
 * userspace into the kernel and checks that each element is valid (e.g.
 * it does not point to a kernel address or cause overflow by being too
 * large, etc.).
 *
 * As an optimization, the caller may provide a pointer to a small
 * on-stack array in @fast_pointer, typically %UIO_FASTIOV elements long
 * (the size of this array, or 0 if unused, should be given in @fast_segs).
 *
 * @ret_pointer will always point to the array that was used, so the
 * caller must take care not to call kfree() on it e.g. in case the
 * @fast_pointer array was used and it was allocated on the stack.
 *
 * Return: The total number of bytes covered by the iovec array on success
 *   or a negative error code on error.
 */
ssize_t
rw_copy_check_uvector(int type, const struct iovec *uvector,
                      unsigned long nr_segs, unsigned long fast_segs,
                      struct iovec *fast_pointer,
                      struct iovec **ret_pointer)
{
    unsigned long seg;
    ssize_t ret;
    struct iovec *iov = fast_pointer;

    /*
     * SuS says "The readv() function *may* fail if the iovcnt argument
     * was less than or equal to 0, or greater than {IOV_MAX}.  Linux has
     * traditionally returned zero for zero segments, so...
     */
    if (nr_segs == 0) {
        ret = 0;
        goto out;
    }

    /*
     * First get the "struct iovec" from user memory and
     * verify all the pointers
     */
    if (nr_segs > UIO_MAXIOV) {
        ret = -EINVAL;
        goto out;
    }
    if (nr_segs > fast_segs) {
        iov = kmalloc_array(nr_segs, sizeof(struct iovec), GFP_KERNEL);
        if (iov == NULL) {
            ret = -ENOMEM;
            goto out;
        }
    }
    if (copy_from_user(iov, uvector, nr_segs*sizeof(*uvector))) {
        ret = -EFAULT;
        goto out;
    }

    /*
     * According to the Single Unix Specification we should return EINVAL
     * if an element length is < 0 when cast to ssize_t or if the
     * total length would overflow the ssize_t return value of the
     * system call.
     *
     * Linux caps all read/write calls to MAX_RW_COUNT, and avoids the
     * overflow case.
     */
    ret = 0;
    for (seg = 0; seg < nr_segs; seg++) {
        void *buf = iov[seg].iov_base;
        ssize_t len = (ssize_t)iov[seg].iov_len;

        /* see if we we're about to use an invalid len or if
         * it's about to overflow ssize_t */
        if (len < 0) {
            ret = -EINVAL;
            goto out;
        }
        if (type >= 0
            && unlikely(!access_ok(buf, len))) {
            ret = -EFAULT;
            goto out;
        }
        if (len > MAX_RW_COUNT - ret) {
            len = MAX_RW_COUNT - ret;
            iov[seg].iov_len = len;
        }
        ret += len;
    }

 out:
    *ret_pointer = iov;
    return ret;
}
EXPORT_SYMBOL(rw_copy_check_uvector);

/**
 * import_iovec() - Copy an array of &struct iovec from userspace
 *     into the kernel, check that it is valid, and initialize a new
 *     &struct iov_iter iterator to access it.
 *
 * @type: One of %READ or %WRITE.
 * @uvector: Pointer to the userspace array.
 * @nr_segs: Number of elements in userspace array.
 * @fast_segs: Number of elements in @iov.
 * @iov: (input and output parameter) Pointer to pointer to (usually small
 *     on-stack) kernel array.
 * @i: Pointer to iterator that will be initialized on success.
 *
 * If the array pointed to by *@iov is large enough to hold all @nr_segs,
 * then this function places %NULL in *@iov on return. Otherwise, a new
 * array will be allocated and the result placed in *@iov. This means that
 * the caller may call kfree() on *@iov regardless of whether the small
 * on-stack array was used or not (and regardless of whether this function
 * returns an error or not).
 *
 * Return: Negative error code on error, bytes imported on success
 */
ssize_t
import_iovec(int type, const struct iovec *uvector,
             unsigned nr_segs, unsigned fast_segs,
             struct iovec **iov, struct iov_iter *i)
{
    ssize_t n;
    struct iovec *p;
    n = rw_copy_check_uvector(type, uvector, nr_segs, fast_segs,
                              *iov, &p);
    if (n < 0) {
        if (p != *iov)
            kfree(p);
        *iov = NULL;
        return n;
    }
    iov_iter_init(i, type, p, nr_segs, n);
    *iov = p == *iov ? NULL : p;
    return n;
}
EXPORT_SYMBOL(import_iovec);

void iov_iter_advance(struct iov_iter *i, size_t size)
{
    if (unlikely(iov_iter_is_pipe(i))) {
        panic("%s: pipe_advance\n", __func__);
        //pipe_advance(i, size);
        return;
    }
    if (unlikely(iov_iter_is_discard(i))) {
        i->count -= size;
        return;
    }
    iterate_and_advance(i, size, v, 0, 0, 0)
}
EXPORT_SYMBOL(iov_iter_advance);

int
init_module(void)
{
    printk("module[iov_iter]: init begin ...\n");
    printk("module[iov_iter]: init end!\n");
    return 0;
}

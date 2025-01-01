/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _LINUX_XARRAY_H
#define _LINUX_XARRAY_H

#include <bug.h>
#include <list.h>
#include <kernel.h>

enum xa_lock_type {
    XA_LOCK_IRQ = 1,
    XA_LOCK_BH  = 2,
};

/*
 * Values for xa_flags.  The radix tree stores its GFP flags in the xa_flags,
 * and we remain compatible with that.
 */
#define XA_FLAGS_LOCK_IRQ   ((gfp_t) XA_LOCK_IRQ)
#define XA_FLAGS_LOCK_BH    ((gfp_t) XA_LOCK_BH)
#define XA_FLAGS_TRACK_FREE ((gfp_t) 4U)
#define XA_FLAGS_ZERO_BUSY  ((gfp_t) 8U)
#define XA_FLAGS_ALLOC_WRAPPED  ((gfp_t) 16U)
#define XA_FLAGS_ACCOUNT    ((gfp_t) 32U)
#define XA_FLAGS_MARK(mark) \
    ((gfp_t)((1U << __GFP_BITS_SHIFT) << (unsigned)(mark)))

#define XA_CHUNK_SHIFT  6
#define XA_CHUNK_SIZE   (1UL << XA_CHUNK_SHIFT)
#define XA_CHUNK_MASK   (XA_CHUNK_SIZE - 1)
#define XA_MAX_MARKS    3
#define XA_MARK_LONGS   DIV_ROUND_UP(XA_CHUNK_SIZE, BITS_PER_LONG)

/*
 * We encode errnos in the xas->xa_node.  If an error has happened, we need to
 * drop the lock to fix it, and once we've done so the xa_state is invalid.
 */
#define XA_ERROR(errno) ((struct xa_node *)(((unsigned long)errno << 2) | 2UL))
#define XAS_BOUNDS  ((struct xa_node *)1UL)
#define XAS_RESTART ((struct xa_node *)3UL)

/**
 * XA_STATE() - Declare an XArray operation state.
 * @name: Name of this operation state (usually xas).
 * @array: Array to operate on.
 * @index: Initial index of interest.
 *
 * Declare and initialise an xa_state on the stack.
 */
#define XA_STATE(name, array, index) \
    struct xa_state name = __XA_STATE(array, index, 0)

struct xa_node {
    unsigned char   shift;  /* Bits remaining in each slot */
    unsigned char   offset; /* Slot offset in parent */

    struct xa_node  *parent;    /* NULL at top of tree */
    struct xarray   *array;     /* The array we belong to */

    void *slots[XA_CHUNK_SIZE];
};

struct xarray {
    gfp_t xa_flags;
    void *xa_head;
};

#define XARRAY_INIT(name, flags) {  \
    .xa_flags = flags,              \
    .xa_head = NULL,                \
}

struct xa_state {
    struct xarray *xa;
    unsigned long xa_index;
    unsigned char xa_shift;
    unsigned char xa_offset;
    struct xa_node *xa_node;
};

#define __XA_STATE(array, index, shift)  {    \
    .xa = array,                    \
    .xa_index = index,              \
    .xa_shift = shift,              \
    .xa_offset = 0,                 \
    .xa_node = XAS_RESTART,         \
}

static inline void *
xa_entry(const struct xarray *xa,
         const struct xa_node *node,
         unsigned int offset)
{
    BUG_ON(offset >= XA_CHUNK_SIZE);
    return node->slots[offset];
}

static inline void *
xa_entry_locked(const struct xarray *xa,
                const struct xa_node *node,
                unsigned int offset)
{
    BUG_ON(offset >= XA_CHUNK_SIZE);
    return node->slots[offset];
}

static inline void *xa_mk_internal(unsigned long v)
{
    return (void *)((v << 2) | 2);
}

static inline bool xa_is_internal(const void *entry)
{
    return ((unsigned long)entry & 3) == 2;
}

static inline bool xa_is_err(const void *entry)
{
    return unlikely(xa_is_internal(entry) &&
                    entry >= xa_mk_internal(-MAX_ERRNO));
}

static inline int xa_err(void *entry)
{
    /* xa_to_internal() would not do sign extension. */
    if (xa_is_err(entry))
        return (long)entry >> 2;
    return 0;
}

static inline int xas_error(const struct xa_state *xas)
{
    return xa_err(xas->xa_node);
}

static inline bool xas_invalid(const struct xa_state *xas)
{
    return (unsigned long)xas->xa_node & 3;
}

static inline bool xas_valid(const struct xa_state *xas)
{
    return !xas_invalid(xas);
}

static inline void *xa_head(const struct xarray *xa)
{
    return xa->xa_head;
}

static inline void *xa_head_locked(const struct xarray *xa)
{
    return xa->xa_head;
}

static inline bool xa_is_node(const void *entry)
{
    return xa_is_internal(entry) && (unsigned long)entry > 4096;
}

#define XA_ZERO_ENTRY   xa_mk_internal(257)

static inline bool xa_is_zero(const void *entry)
{
    return unlikely(entry == XA_ZERO_ENTRY);
}

static inline bool xas_top(struct xa_node *node)
{
    return node <= XAS_RESTART;
}

static inline struct xa_node *xa_to_node(const void *entry)
{
    return (struct xa_node *)((unsigned long)entry - 2);
}

static inline void *xa_mk_node(const struct xa_node *node)
{
    return (void *)((unsigned long)node | 2);
}

void *xas_load(struct xa_state *xas);

void *xas_store(struct xa_state *xas, void *entry);

bool xas_nomem(struct xa_state *xas, gfp_t gfp);

static inline void xas_set_err(struct xa_state *xas, long err)
{
    xas->xa_node = XA_ERROR(err);
}

void *xa_load(struct xarray *xa, unsigned long index);

void *xas_find(struct xa_state *, unsigned long max);

/* True if the pointer is something other than a node */
static inline bool xas_not_node(struct xa_node *node)
{
    return ((unsigned long)node & 3) || !node;
}

static inline void *
xas_next_entry(struct xa_state *xas, unsigned long max)
{
    struct xa_node *node = xas->xa_node;
    void *entry;

    if (unlikely(xas_not_node(node) || node->shift ||
            xas->xa_offset != (xas->xa_index & XA_CHUNK_MASK)))
        return xas_find(xas, max);

    do {
        if (unlikely(xas->xa_index >= max))
            return xas_find(xas, max);
        if (unlikely(xas->xa_offset == XA_CHUNK_MASK))
            return xas_find(xas, max);
        entry = xa_entry(xas->xa, node, xas->xa_offset + 1);
        if (unlikely(xa_is_internal(entry)))
            return xas_find(xas, max);
        xas->xa_offset++;
        xas->xa_index++;
    } while (!entry);

    return entry;
}

/**
 * xas_for_each() - Iterate over a range of an XArray.
 * @xas: XArray operation state.
 * @entry: Entry retrieved from the array.
 * @max: Maximum index to retrieve from array.
 *
 * The loop body will be executed for each entry present in the xarray
 * between the current xas position and @max.  @entry will be set to
 * the entry retrieved from the xarray.  It is safe to delete entries
 * from the array in the loop body.  You should hold either the RCU lock
 * or the xa_lock while iterating.  If you need to drop the lock, call
 * xas_pause() first.
 */
#define xas_for_each(xas, entry, max)       \
    for (entry = xas_find(xas, max); entry; \
         entry = xas_next_entry(xas, max))

static inline struct xa_node *
xa_parent(const struct xarray *xa, const struct xa_node *node)
{
    return node->parent;
}

/**
 * xa_init_flags() - Initialise an empty XArray with flags.
 * @xa: XArray.
 * @flags: XA_FLAG values.
 *
 * If you need to initialise an XArray with special flags (eg you need
 * to take the lock from interrupt context), use this function instead
 * of xa_init().
 *
 * Context: Any context.
 */
static inline void xa_init_flags(struct xarray *xa, gfp_t flags)
{
    xa->xa_flags = flags;
    xa->xa_head = NULL;
}

#endif /* _LINUX_XARRAY_H */

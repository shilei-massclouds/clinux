// SPDX-License-Identifier: GPL-2.0-only

#include <slab.h>
#include <errno.h>
#include <export.h>
#include <printk.h>
#include <string.h>
#include <xarray.h>

struct kmem_cache *xa_node_cachep;

static void *set_bounds(struct xa_state *xas)
{
    xas->xa_node = XAS_BOUNDS;
    return NULL;
}

static void *xas_start(struct xa_state *xas)
{
    void *entry;

    if (xas_valid(xas))
        panic("xas valid!");
    if (xas_error(xas))
        return NULL;

    entry = xa_head(xas->xa);
    if (!xa_is_node(entry)) {
        if (xas->xa_index)
            return set_bounds(xas);
    } else {
        if ((xas->xa_index >> xa_to_node(entry)->shift) > XA_CHUNK_MASK)
            return set_bounds(xas);
    }

    xas->xa_node = NULL;
    return entry;
}

/* extracts the offset within this node from the index */
static unsigned int get_offset(unsigned long index, struct xa_node *node)
{
    return (index >> node->shift) & XA_CHUNK_MASK;
}

static void *xas_descend(struct xa_state *xas, struct xa_node *node)
{
    unsigned int offset = get_offset(xas->xa_index, node);
    void *entry = xa_entry(xas->xa, node, offset);

    xas->xa_node = node;
    xas->xa_offset = offset;
    return entry;
}

void *xas_load(struct xa_state *xas)
{
    void *entry = xas_start(xas);

    while (xa_is_node(entry)) {
        struct xa_node *node = xa_to_node(entry);

        if (xas->xa_shift > node->shift)
            break;
        entry = xas_descend(xas, node);
        if (node->shift == 0)
            break;
    }
    return entry;
}
EXPORT_SYMBOL(xas_load);

/**
 * xa_load() - Load an entry from an XArray.
 * @xa: XArray.
 * @index: index into array.
 *
 * Context: Any context.  Takes and releases the RCU lock.
 * Return: The entry at @index in @xa.
 */
void *xa_load(struct xarray *xa, unsigned long index)
{
    void *entry;
    XA_STATE(xas, xa, index);

    entry = xas_load(&xas);
    if (xa_is_zero(entry))
        entry = NULL;

    return entry;
}
EXPORT_SYMBOL(xa_load);

/* The maximum index that can be contained in the array without expanding it */
static unsigned long max_index(void *entry)
{
    if (!xa_is_node(entry))
        return 0;
    return (XA_CHUNK_SIZE << xa_to_node(entry)->shift) - 1;
}

static void *xas_alloc(struct xa_state *xas, unsigned int shift)
{
    struct xa_node *node = NULL;
    struct xa_node *parent = xas->xa_node;
    gfp_t gfp = GFP_NOWAIT|__GFP_NOWARN;

    if (xas_invalid(xas))
        return NULL;

    node = kmem_cache_alloc(xa_node_cachep, gfp);
    if (!node) {
        xas_set_err(xas, -ENOMEM);
        return NULL;
    }

    if (parent) {
        node->offset = xas->xa_offset;
    }

    BUG_ON(shift > BITS_PER_LONG);
    node->shift = shift;
    node->parent = xas->xa_node;
    node->array = xas->xa;
    return node;
}

/*
 * xas_expand adds nodes to the head of the tree until it has reached
 * sufficient height to be able to contain @xas->xa_index
 */
static int xas_expand(struct xa_state *xas, void *head)
{
    unsigned int shift = 0;
    struct xa_node *node = NULL;
    struct xarray *xa = xas->xa;

    if (!head) {
        if (xas->xa_index == 0)
            return 0;
        while ((xas->xa_index >> shift) >= XA_CHUNK_SIZE)
            shift += XA_CHUNK_SHIFT;
        return shift + XA_CHUNK_SHIFT;
    } else if (xa_is_node(head)) {
        node = xa_to_node(head);
        shift = node->shift + XA_CHUNK_SHIFT;
    }
    xas->xa_node = NULL;

    while (xas->xa_index > max_index(head)) {
        BUG_ON(shift > BITS_PER_LONG);
        node = xas_alloc(xas, shift);
        if (!node)
            return -ENOMEM;

        node->slots[0] = head;

        if (xa_is_node(head)) {
            xa_to_node(head)->offset = 0;
            xa_to_node(head)->parent = node;
        }
        head = xa_mk_node(node);
        xa->xa_head = head;

        shift += XA_CHUNK_SHIFT;
    }

    xas->xa_node = node;
    return shift;
}

static void *xas_create(struct xa_state *xas, bool allow_root)
{
    int shift;
    void *entry;
    void **slot;
    struct xarray *xa = xas->xa;
    struct xa_node *node = xas->xa_node;
    unsigned int order = xas->xa_shift;

    if (xas_top(node)) {
        entry = xa_head_locked(xa);
        xas->xa_node = NULL;
        shift = xas_expand(xas, entry);
        if (shift < 0)
            return NULL;
        if (!shift && !allow_root)
            shift = XA_CHUNK_SHIFT;
        entry = xa_head_locked(xa);
        slot = &xa->xa_head;
    } else if (xas_error(xas)) {
        return NULL;
    } else if (node) {
        unsigned int offset = xas->xa_offset;

        shift = node->shift;
        entry = xa_entry_locked(xa, node, offset);
        slot = &node->slots[offset];
    } else {
        shift = 0;
        entry = xa_head_locked(xa);
        slot = &xa->xa_head;
    }

    while (shift > order) {
        shift -= XA_CHUNK_SHIFT;
        if (!entry) {
            node = xas_alloc(xas, shift);
            if (!node)
                break;
            *slot = xa_mk_node(node);
        } else if (xa_is_node(entry)) {
            node = xa_to_node(entry);
        } else {
            break;
        }
        entry = xas_descend(xas, node);
        slot = &node->slots[xas->xa_offset];
    }

    return entry;
}

void *xas_store(struct xa_state *xas, void *entry)
{
    void *first;
    struct xa_node *node;
    void **slot = &xas->xa->xa_head;

    if (entry) {
        bool allow_root = !xa_is_node(entry) && !xa_is_zero(entry);
        first = xas_create(xas, allow_root);
    } else {
        first = xas_load(xas);
    }

    if (xas_invalid(xas))
        return first;

    node = xas->xa_node;
    if (first == entry)
        return first;

    if (node)
        slot = &node->slots[xas->xa_offset];

    *slot = entry;

    return first;
}
EXPORT_SYMBOL(xas_store);

static void
xa_node_ctor(void *arg)
{
    struct xa_node *node = arg;

    memset(node, 0, sizeof(*node));
}

/* move the index either forwards (find) or backwards (sibling slot) */
static void xas_move_index(struct xa_state *xas, unsigned long offset)
{
    unsigned int shift = xas->xa_node->shift;
    xas->xa_index &= ~XA_CHUNK_MASK << shift;
    xas->xa_index += offset << shift;
}

static void xas_advance(struct xa_state *xas)
{
    xas->xa_offset++;
    xas_move_index(xas, xas->xa_offset);
}

void *xas_find(struct xa_state *xas, unsigned long max)
{
    void *entry;

    if (xas_error(xas) || xas->xa_node == XAS_BOUNDS)
        return NULL;
    if (xas->xa_index > max)
        return set_bounds(xas);

    if (!xas->xa_node) {
        xas->xa_index = 1;
        return set_bounds(xas);
    } else if (xas->xa_node == XAS_RESTART) {
        entry = xas_load(xas);
        if (entry || xas_not_node(xas->xa_node))
            return entry;
    } else if (!xas->xa_node->shift &&
            xas->xa_offset != (xas->xa_index & XA_CHUNK_MASK)) {
        xas->xa_offset = ((xas->xa_index - 1) & XA_CHUNK_MASK) + 1;
    }

    xas_advance(xas);

    while (xas->xa_node && (xas->xa_index <= max)) {
        if (unlikely(xas->xa_offset == XA_CHUNK_SIZE)) {
            xas->xa_offset = xas->xa_node->offset + 1;
            xas->xa_node = xa_parent(xas->xa, xas->xa_node);
            continue;
        }

        entry = xa_entry(xas->xa, xas->xa_node, xas->xa_offset);
        if (xa_is_node(entry)) {
            xas->xa_node = xa_to_node(entry);
            xas->xa_offset = 0;
            continue;
        }
        if (entry)
            return entry;

        xas_advance(xas);
    }

    if (!xas->xa_node)
        xas->xa_node = XAS_BOUNDS;
    return NULL;
}
EXPORT_SYMBOL(xas_find);

void xarray_init(void)
{
    BUG_ON(XA_CHUNK_SIZE > 255);
    xa_node_cachep = kmem_cache_create("xa_node",
                                       sizeof(struct xa_node), 0,
                                       SLAB_PANIC | SLAB_RECLAIM_ACCOUNT,
                                       xa_node_ctor);
}

int
init_module(void)
{
    printk("module[xarray]: init begin ...\n");

    xarray_init();

    printk("module[xarray]: init end!\n");
    return 0;
}

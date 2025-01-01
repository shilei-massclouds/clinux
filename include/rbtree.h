/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _LINUX_RBTREE_H_
#define _LINUX_RBTREE_H_

#include <list.h>
#include <atomic.h>

struct rb_node {
    unsigned long  __rb_parent_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));

struct rb_root {
    struct rb_node *rb_node;
};

/*
 * Leftmost-cached rbtrees.
 *
 * We do not cache the rightmost node based on footprint
 * size vs number of potential users that could benefit
 * from O(1) rb_last(). Just not worth it, users that want
 * this feature can always implement the logic explicitly.
 * Furthermore, users that want to cache both pointers may
 * find it a bit asymmetric, but that's ok.
 */
struct rb_root_cached {
    struct rb_root rb_root;
    struct rb_node *rb_leftmost;
};

#define RB_ROOT_CACHED (struct rb_root_cached) { {NULL, }, NULL }

/* Same as rb_first(), but O(1) */
#define rb_first_cached(root)   (root)->rb_leftmost

struct rb_augment_callbacks {
    void (*propagate)(struct rb_node *node, struct rb_node *stop);
    void (*copy)(struct rb_node *old, struct rb_node *new);
    void (*rotate)(struct rb_node *old, struct rb_node *new);
};

#define RB_RED      0
#define RB_BLACK    1

#define rb_parent(r) ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define __rb_parent(pc)     ((struct rb_node *)(pc & ~3))

#define __rb_color(pc)      ((pc) & 1)
#define __rb_is_red(pc)     (!__rb_color(pc))
#define __rb_is_black(pc)   __rb_color(pc)
#define rb_color(rb)        __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)       __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)     __rb_is_black((rb)->__rb_parent_color)

#define RB_ROOT (struct rb_root) { NULL, }

#define RB_EMPTY_ROOT(root) (READ_ONCE((root)->rb_node) == NULL)

#define RB_EMPTY_NODE(node) \
    ((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node) \
    ((node)->__rb_parent_color = (unsigned long)(node))

#define rb_entry(ptr, type, member) container_of(ptr, type, member)

#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
        ____ptr ? rb_entry(____ptr, type, member) : NULL; \
     })

void rb_insert_color(struct rb_node *, struct rb_root *);
void rb_erase(struct rb_node *, struct rb_root *);

static inline void
rb_link_node(struct rb_node *node,
             struct rb_node *parent,
             struct rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
}

static inline void
rb_set_parent_color(struct rb_node *rb, struct rb_node *p, int color)
{
    rb->__rb_parent_color = (unsigned long)p | color;
}

static inline void
__rb_change_child(struct rb_node *old, struct rb_node *new,
                  struct rb_node *parent, struct rb_root *root)
{
    if (parent) {
        if (parent->rb_left == old)
            WRITE_ONCE(parent->rb_left, new);
        else
            WRITE_ONCE(parent->rb_right, new);
    } else {
        WRITE_ONCE(root->rb_node, new);
    }
}

static inline void
rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
    rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

void
__rb_insert_augmented(struct rb_node *node,
                      struct rb_root *root,
                      void (*augment_rotate)(struct rb_node *old,
                                             struct rb_node *new));

/*
 * Fixup the rbtree and update the augmented information when rebalancing.
 *
 * On insertion, the user must update the augmented information on the path
 * leading to the inserted node, then call rb_link_node() as usual and
 * rb_insert_augmented() instead of the usual rb_insert_color() call.
 * If rb_insert_augmented() rebalances the rbtree, it will callback into
 * a user provided function to update the augmented information on the
 * affected subtrees.
 */
static inline void
rb_insert_augmented(struct rb_node *node,
                    struct rb_root *root,
                    const struct rb_augment_callbacks *augment)
{
    __rb_insert_augmented(node, root, augment->rotate);
}

struct rb_node *
__rb_erase_augmented(struct rb_node *node,
                     struct rb_root *root,
                     const struct rb_augment_callbacks *augment);

void
__rb_erase_color(struct rb_node *parent,
                 struct rb_root *root,
                 void (*augment_rotate)(struct rb_node *old,
                                        struct rb_node *new));

static __always_inline void
rb_erase_augmented(struct rb_node *node,
                   struct rb_root *root,
                   const struct rb_augment_callbacks *augment)
{
    struct rb_node *rebalance = __rb_erase_augmented(node, root, augment);
    if (rebalance)
        __rb_erase_color(rebalance, root, augment->rotate);
}

/*
 * Template for declaring augmented rbtree callbacks (generic case)
 *
 * RBSTATIC:    'static' or empty
 * RBNAME:      name of the rb_augment_callbacks structure
 * RBSTRUCT:    struct type of the tree nodes
 * RBFIELD:     name of struct rb_node field within RBSTRUCT
 * RBAUGMENTED: name of field within RBSTRUCT holding data for subtree
 * RBCOMPUTE:   name of function that recomputes the RBAUGMENTED data
 */

#define RB_DECLARE_CALLBACKS(RBSTATIC, RBNAME,              \
                 RBSTRUCT, RBFIELD, RBAUGMENTED, RBCOMPUTE) \
static inline void                          \
RBNAME ## _propagate(struct rb_node *rb, struct rb_node *stop)      \
{                                   \
    while (rb != stop) {                        \
        RBSTRUCT *node = rb_entry(rb, RBSTRUCT, RBFIELD);   \
        if (RBCOMPUTE(node, true))              \
            break;                      \
        rb = rb_parent(&node->RBFIELD);             \
    }                               \
}                                   \
static inline void                          \
RBNAME ## _copy(struct rb_node *rb_old, struct rb_node *rb_new)     \
{                                   \
    RBSTRUCT *old = rb_entry(rb_old, RBSTRUCT, RBFIELD);        \
    RBSTRUCT *new = rb_entry(rb_new, RBSTRUCT, RBFIELD);        \
    new->RBAUGMENTED = old->RBAUGMENTED;                \
}                                   \
static void                             \
RBNAME ## _rotate(struct rb_node *rb_old, struct rb_node *rb_new)   \
{                                   \
    RBSTRUCT *old = rb_entry(rb_old, RBSTRUCT, RBFIELD);        \
    RBSTRUCT *new = rb_entry(rb_new, RBSTRUCT, RBFIELD);        \
    new->RBAUGMENTED = old->RBAUGMENTED;                \
    RBCOMPUTE(old, false);                      \
}                                   \
RBSTATIC const struct rb_augment_callbacks RBNAME = {           \
    .propagate = RBNAME ## _propagate,              \
    .copy = RBNAME ## _copy,                    \
    .rotate = RBNAME ## _rotate                 \
};

/*
 * Template for declaring augmented rbtree callbacks,
 * computing RBAUGMENTED scalar as max(RBCOMPUTE(node)) for all subtree nodes.
 *
 * RBSTATIC:    'static' or empty
 * RBNAME:      name of the rb_augment_callbacks structure
 * RBSTRUCT:    struct type of the tree nodes
 * RBFIELD:     name of struct rb_node field within RBSTRUCT
 * RBTYPE:      type of the RBAUGMENTED field
 * RBAUGMENTED: name of RBTYPE field within RBSTRUCT holding data for subtree
 * RBCOMPUTE:   name of function that returns the per-node RBTYPE scalar
 */

#define RB_DECLARE_CALLBACKS_MAX(RBSTATIC, RBNAME, RBSTRUCT, RBFIELD,         \
                 RBTYPE, RBAUGMENTED, RBCOMPUTE)          \
static inline bool RBNAME ## _compute_max(RBSTRUCT *node, bool exit)          \
{                                         \
    RBSTRUCT *child;                              \
    RBTYPE max = RBCOMPUTE(node);                         \
    if (node->RBFIELD.rb_left) {                          \
        child = rb_entry(node->RBFIELD.rb_left, RBSTRUCT, RBFIELD);   \
        if (child->RBAUGMENTED > max)                     \
            max = child->RBAUGMENTED;                 \
    }                                     \
    if (node->RBFIELD.rb_right) {                         \
        child = rb_entry(node->RBFIELD.rb_right, RBSTRUCT, RBFIELD);  \
        if (child->RBAUGMENTED > max)                     \
            max = child->RBAUGMENTED;                 \
    }                                     \
    if (exit && node->RBAUGMENTED == max)                     \
        return true;                              \
    node->RBAUGMENTED = max;                          \
    return false;                                 \
}                                         \
RB_DECLARE_CALLBACKS(RBSTATIC, RBNAME,                        \
             RBSTRUCT, RBFIELD, RBAUGMENTED, RBNAME ## _compute_max)

struct rb_node *rb_first(const struct rb_root *root);
struct rb_node *rb_next(const struct rb_node *node);

static inline void
rb_insert_color_cached(struct rb_node *node,
                       struct rb_root_cached *root,
                       bool leftmost)
{
    if (leftmost)
        root->rb_leftmost = node;
    rb_insert_color(node, &root->rb_root);
}

static inline void
rb_erase_cached(struct rb_node *node, struct rb_root_cached *root)
{
    if (root->rb_leftmost == node)
        root->rb_leftmost = rb_next(node);
    rb_erase(node, &root->rb_root);
}


#endif /* _LINUX_RBTREE_H_ */

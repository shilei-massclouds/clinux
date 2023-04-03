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

extern struct rb_node *rb_next(const struct rb_node *);
extern struct rb_node *rb_prev(const struct rb_node *);
extern struct rb_node *rb_first(const struct rb_root *);
extern struct rb_node *rb_last(const struct rb_root *);

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

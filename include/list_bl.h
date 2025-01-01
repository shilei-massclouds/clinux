/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LIST_BL_H
#define _LINUX_LIST_BL_H

#define LIST_BL_LOCKMASK    1UL

#define hlist_bl_entry(ptr, type, member) container_of(ptr, type, member)

struct hlist_bl_head {
    struct hlist_bl_node *first;
};

struct hlist_bl_node {
    struct hlist_bl_node *next, **pprev;
};

static inline void
INIT_HLIST_BL_NODE(struct hlist_bl_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

static inline bool
hlist_bl_unhashed(const struct hlist_bl_node *h)
{
    return !h->pprev;
}

static inline struct hlist_bl_node *
hlist_bl_first(struct hlist_bl_head *h)
{
    return (struct hlist_bl_node *)
        ((unsigned long)h->first & ~LIST_BL_LOCKMASK);
}

static inline void
hlist_bl_set_first(struct hlist_bl_head *h, struct hlist_bl_node *n)
{
    h->first = (struct hlist_bl_node *)((unsigned long)n | LIST_BL_LOCKMASK);
}

static inline void
hlist_bl_add_head(struct hlist_bl_node *n, struct hlist_bl_head *h)
{
    struct hlist_bl_node *first = hlist_bl_first(h);

    n->next = first;
    if (first)
        first->pprev = &n->next;
    n->pprev = &h->first;
    hlist_bl_set_first(h, n);
}

/**
 * hlist_bl_for_each_entry  - iterate over list of given type
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 *
 */
#define hlist_bl_for_each_entry(tpos, pos, head, member)    \
    for (pos = hlist_bl_first(head);                        \
         pos &&                                             \
         ({ tpos = hlist_bl_entry(pos, typeof(*tpos), member); 1;}); \
         pos = pos->next)

static inline void __hlist_bl_del(struct hlist_bl_node *n)
{
    struct hlist_bl_node *next = n->next;
    struct hlist_bl_node **pprev = n->pprev;

    BUG_ON((unsigned long)n & LIST_BL_LOCKMASK);

    /* pprev may be `first`, so be careful not to lose the lock bit */
    WRITE_ONCE(*pprev, (struct hlist_bl_node *)
               ((unsigned long)next |
                ((unsigned long)*pprev & LIST_BL_LOCKMASK)));
    if (next)
        next->pprev = pprev;
}

#endif /* _LINUX_LIST_BL_H */

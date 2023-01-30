// SPDX-License-Identifier: GPL-2.0
#include <klist.h>
#include <export.h>

static void
knode_set_klist(struct klist_node *knode, struct klist *klist)
{
    knode->n_klist = klist;
}

static void
klist_node_init(struct klist *k, struct klist_node *n)
{
    INIT_LIST_HEAD(&n->n_node);
    knode_set_klist(n, k);
}

static void
add_tail(struct klist *k, struct klist_node *n)
{
    list_add_tail(&n->n_node, &k->k_list);
}

void
klist_add_tail(struct klist_node *n, struct klist *k)
{
    klist_node_init(k, n);
    add_tail(k, n);
}
EXPORT_SYMBOL(klist_add_tail);

void
klist_init(struct klist *k)
{
    INIT_LIST_HEAD(&k->k_list);
}
EXPORT_SYMBOL(klist_init);

void
klist_iter_init_node(struct klist *k,
                     struct klist_iter *i,
                     struct klist_node *n)
{
    i->i_klist = k;
    i->i_cur = NULL;
    if (n)
        i->i_cur = n;
}
EXPORT_SYMBOL(klist_iter_init_node);

static struct klist_node *
to_klist_node(struct list_head *n)
{
    return container_of(n, struct klist_node, n_node);
}

static int klist_dec_and_del(struct klist_node *n)
{
    return 1;
}

#define KNODE_DEAD      1LU
#define KNODE_KLIST_MASK    ~KNODE_DEAD

static bool
knode_dead(struct klist_node *knode)
{
    return (unsigned long)knode->n_klist & KNODE_DEAD;
}

struct klist_node *
klist_next(struct klist_iter *i)
{
    struct klist_node *next;
    struct klist_node *last = i->i_cur;
    unsigned long flags;

    if (last) {
        next = to_klist_node(last->n_node.next);
        klist_dec_and_del(last);
    } else {
        next = to_klist_node(i->i_klist->k_list.next);
    }

    i->i_cur = NULL;
    while (next != to_klist_node(&i->i_klist->k_list)) {
        if (likely(!knode_dead(next))) {
            i->i_cur = next;
            break;
        }
        next = to_klist_node(next->n_node.next);
    }

    return i->i_cur;
}
EXPORT_SYMBOL(klist_next);

void
klist_iter_exit(struct klist_iter *i)
{
    if (i->i_cur) {
        i->i_cur = NULL;
    }
}
EXPORT_SYMBOL(klist_iter_exit);

int
klist_node_attached(struct klist_node *n)
{
    return (n->n_klist != NULL);
}
EXPORT_SYMBOL(klist_node_attached);

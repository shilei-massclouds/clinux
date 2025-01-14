/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _LINUX_KLIST_H
#define _LINUX_KLIST_H

#include <list.h>

struct klist_node {
    void *n_klist;   /* never access directly */
    struct list_head n_node;
};

struct klist {
    struct list_head k_list;
} __attribute__ ((aligned (sizeof(void *))));

struct klist_iter {
    struct klist        *i_klist;
    struct klist_node   *i_cur;
};

extern void klist_init(struct klist *k);

extern void klist_add_tail(struct klist_node *n, struct klist *k);

void
klist_iter_init_node(struct klist *k,
                     struct klist_iter *i,
                     struct klist_node *n);

struct klist_node *
klist_next(struct klist_iter *i);

void
klist_iter_exit(struct klist_iter *i);

int
klist_node_attached(struct klist_node *n);

#endif /* _LINUX_KLIST_H */

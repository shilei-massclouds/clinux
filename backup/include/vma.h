// SPDX-License-Identifier: GPL-2.0-only
#ifndef _VMA_H_
#define _VMA_H_

#include <rbtree.h>

#define VM_IOREMAP          0x00000001  /* ioremap() and friends */
#define VM_ALLOC            0x00000002  /* vmalloc() */
#define VM_UNINITIALIZED    0x00000020  /* vm_struct is not fully initialized */
#define VM_NO_GUARD         0x00000040  /* don't add guard page */

#define IOREMAP_MAX_ORDER   (7 + PAGE_SHIFT)    /* 128 pages */

struct vm_struct {
    void            *addr;
    unsigned long   size;
    unsigned long   flags;
};

struct vmap_area {
    unsigned long va_start;
    unsigned long va_end;

    struct rb_node rb_node;         /* address sorted rbtree */
    struct list_head list;          /* address sorted list */

    union {
        unsigned long subtree_max_size; /* in "free" tree */
        struct vm_struct *vm;           /* in "busy" tree */
    };
};

struct vm_struct *
get_vm_area(unsigned long size, unsigned long flags);

void
free_vm_area(struct vm_struct *area);

#endif /* _VMA_H_ */

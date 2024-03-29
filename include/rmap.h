/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RMAP_H
#define _LINUX_RMAP_H

#include <mm.h>

/*
 * The anon_vma heads a list of private "related" vmas, to scan if
 * an anonymous page pointing to this anon_vma needs to be unmapped:
 * the vmas on the list will be related by forking, or by splitting.
 *
 * Since vmas come and go as they are split and merged (particularly
 * in mprotect), the mapping field of an anonymous page cannot point
 * directly to a vma: instead it points to an anon_vma, on whose list
 * the related vmas can be easily linked or unlinked.
 *
 * After unlinking the last vma on the list, we must garbage collect
 * the anon_vma object itself: we're guaranteed no page can be
 * pointing to this anon_vma once its vma list is empty.
 */
struct anon_vma {
    struct anon_vma *root;      /* Root of this anon_vma tree */
    /*
     * The refcount is taken on an anon_vma when there is no
     * guarantee that the vma of page tables will exist for
     * the duration of the operation. A caller that takes
     * the reference is responsible for clearing up the
     * anon_vma if they are the last user on release
     */
    atomic_t refcount;

    /*
     * Count of child anon_vmas and VMAs which points to this anon_vma.
     *
     * This counter is used for making decision about reusing anon_vma
     * instead of forking new one. See comments in function anon_vma_clone.
     */
    unsigned degree;

    struct anon_vma *parent;    /* Parent of this anon_vma */

    /*
     * NOTE: the LSB of the rb_root.rb_node is set by
     * mm_take_all_locks() _after_ taking the above lock. So the
     * rb_root must only be read/written after taking the above lock
     * to be sure to see a valid next pointer. The LSB bit itself
     * is serialized by a system wide lock only visible to
     * mm_take_all_locks() (mm_all_locks_mutex).
     */

    /* Interval tree of private "related" vmas */
    struct rb_root_cached rb_root;
};

/*
 * The copy-on-write semantics of fork mean that an anon_vma
 * can become associated with multiple processes. Furthermore,
 * each child process will have its own anon_vma, where new
 * pages for that process are instantiated.
 *
 * This structure allows us to find the anon_vmas associated
 * with a VMA, or the VMAs associated with an anon_vma.
 * The "same_vma" list contains the anon_vma_chains linking
 * all the anon_vmas associated with this VMA.
 * The "rb" field indexes on an interval tree the anon_vma_chains
 * which link all the VMAs associated with this anon_vma.
 */
struct anon_vma_chain {
    struct vm_area_struct *vma;
    struct anon_vma *anon_vma;
    struct list_head same_vma;   /* locked by mmap_lock & page_table_lock */
    struct rb_node rb;          /* locked by anon_vma->rwsem */
    unsigned long rb_subtree_last;
};

static inline int anon_vma_prepare(struct vm_area_struct *vma)
{
    /*
    if (likely(vma->anon_vma))
        return 0;
        */

    panic("no anon_vma!");
    //return __anon_vma_prepare(vma);
}

#endif  /* _LINUX_RMAP_H */

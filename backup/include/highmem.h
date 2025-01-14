/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_HIGHMEM_H
#define _LINUX_HIGHMEM_H

#include <mm.h>
#include <mm_types.h>

static inline void *kmap_atomic(struct page *page)
{
    return page_address(page);
}
#define kmap_atomic_prot(page, prot)    kmap_atomic(page)

/*
 * Prevent people trying to call kunmap_atomic() as if it were kunmap()
 * kunmap_atomic() should get the return value of kmap_atomic, not the page.
 */
#define kunmap_atomic(addr)

static inline void
clear_user_highpage(struct page *page, unsigned long vaddr)
{
    void *addr = kmap_atomic(page);
    clear_user_page(addr, vaddr, page);
    kunmap_atomic(addr);
}

static inline struct page *
__alloc_zeroed_user_highpage(gfp_t movableflags,
                             struct vm_area_struct *vma,
                             unsigned long vaddr)
{
    struct page *page = alloc_page_vma(GFP_HIGHUSER | movableflags,
                                       vma, vaddr);

    if (page)
        clear_user_highpage(page, vaddr);

    return page;
}

/**
 * alloc_zeroed_user_highpage_movable - Allocate a zeroed HIGHMEM page
 * for a VMA that the caller knows can move
 * @vma: The VMA the page is to be allocated for
 * @vaddr: The virtual address the page will be inserted into
 *
 * This function will allocate a page for a VMA that the caller knows
 * will be able to migrate in the future using move_pages() or reclaimed
 */
static inline struct page *
alloc_zeroed_user_highpage_movable(struct vm_area_struct *vma,
                                   unsigned long vaddr)
{
    return __alloc_zeroed_user_highpage(__GFP_MOVABLE, vma, vaddr);
}

static inline void *kmap(struct page *page)
{
    return page_address(page);
}

static inline void
copy_user_highpage(struct page *to, struct page *from,
                   unsigned long vaddr, struct vm_area_struct *vma)
{
    char *vfrom, *vto;

    vfrom = kmap_atomic(from);
    vto = kmap_atomic(to);
    copy_user_page(vto, vfrom, vaddr, to);
    kunmap_atomic(vto);
    kunmap_atomic(vfrom);
}

static inline void clear_highpage(struct page *page)
{
    void *kaddr = kmap_atomic(page);
    clear_page(kaddr);
    kunmap_atomic(kaddr);
}

static inline void
zero_user_segments(struct page *page,
                   unsigned start1, unsigned end1,
                   unsigned start2, unsigned end2)
{
    void *kaddr = kmap_atomic(page);

    BUG_ON(end1 > PAGE_SIZE || end2 > PAGE_SIZE);

    if (end1 > start1)
        memset(kaddr + start1, 0, end1 - start1);

    if (end2 > start2)
        memset(kaddr + start2, 0, end2 - start2);

    kunmap_atomic(kaddr);
    //flush_dcache_page(page);
}

static inline void
zero_user_segment(struct page *page, unsigned start, unsigned end)
{
    zero_user_segments(page, start, end, 0, 0);
}

#endif /* _LINUX_HIGHMEM_H */

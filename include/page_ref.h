/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PAGE_REF_H
#define _LINUX_PAGE_REF_H

static inline void set_page_count(struct page *page, int v)
{
    atomic_set(&page->_refcount, v);
}

/*
 * Turn a non-refcounted page (->_refcount == 0) into refcounted with
 * a count of one.
 */
static inline void set_page_refcounted(struct page *page)
{
    set_page_count(page, 1);
}

static inline int page_ref_dec_and_test(struct page *page)
{
    return atomic_dec_and_test(&page->_refcount);
}

/*
 * Drop a ref, return true if the refcount fell to zero (the page has no users)
 */
static inline int put_page_testzero(struct page *page)
{
    return page_ref_dec_and_test(page);
}

#endif /* _LINUX_PAGE_REF_H */

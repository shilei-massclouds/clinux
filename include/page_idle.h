/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_PAGE_IDLE_H
#define _LINUX_MM_PAGE_IDLE_H

#include <bitops.h>
#include <page-flags.h>

static inline bool page_is_young(struct page *page)
{
    return false;
}

static inline void set_page_young(struct page *page)
{
}

static inline bool test_and_clear_page_young(struct page *page)
{
    return false;
}

static inline bool page_is_idle(struct page *page)
{
    return false;
}

static inline void set_page_idle(struct page *page)
{
}

static inline void clear_page_idle(struct page *page)
{
}

#endif /* _LINUX_MM_PAGE_IDLE_H */

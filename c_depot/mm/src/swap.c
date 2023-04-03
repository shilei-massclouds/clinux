// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <export.h>

/*
 * Mark a page as having seen activity.
 *
 * inactive,unreferenced    ->  inactive,referenced
 * inactive,referenced      ->  active,unreferenced
 * active,unreferenced      ->  active,referenced
 *
 * When a newly allocated page is not yet visible, so safe for non-atomic ops,
 * __SetPageReferenced(page) may be substituted for mark_page_accessed(page).
 */
void mark_page_accessed(struct page *page)
{
    pr_err("%s: todo!\n", __func__);
}
EXPORT_SYMBOL(mark_page_accessed);

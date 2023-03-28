// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <export.h>

int set_page_dirty(struct page *page)
{
    if (!PageDirty(page)) {
        if (!TestSetPageDirty(page))
            return 1;
    }
    return 0;
}
EXPORT_SYMBOL(set_page_dirty);

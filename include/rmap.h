/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_RMAP_H
#define _LINUX_RMAP_H

#include <mm.h>

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

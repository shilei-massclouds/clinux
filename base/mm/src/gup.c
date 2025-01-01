// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <export.h>

int
__mm_populate(unsigned long start, unsigned long len, int ignore_errors)
{
    panic("%s: !", __func__);
}
EXPORT_SYMBOL(__mm_populate);

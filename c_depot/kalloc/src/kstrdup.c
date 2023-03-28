// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <string.h>
#include <slab.h>

void
kfree_const(const void *x)
{
}
EXPORT_SYMBOL(kfree_const);

char *
kstrdup(const char *s, gfp_t gfp)
{
    size_t len;
    char *buf;

    if (!s)
        return NULL;

    len = strlen(s) + 1;
    buf = kmalloc(len, gfp);
    if (buf)
        memcpy(buf, s, len);
    return buf;
}
EXPORT_SYMBOL(kstrdup);

const char *
kstrdup_const(const char *s, gfp_t gfp)
{
    return kstrdup(s, gfp);
}
EXPORT_SYMBOL(kstrdup_const);

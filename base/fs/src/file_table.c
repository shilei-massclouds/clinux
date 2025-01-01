// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <slab.h>
#include <errno.h>

/* SLAB cache for file structures */
static struct kmem_cache *filp_cachep;

static struct file *__alloc_file(int flags, const struct cred *cred)
{
    struct file *f;

    f = kmem_cache_zalloc(filp_cachep, GFP_KERNEL);
    if (unlikely(!f))
        return ERR_PTR(-ENOMEM);

    f->f_flags = flags;
    f->f_mode = OPEN_FMODE(flags);
    /* f->f_version: 0 */

    return f;
}

struct file *alloc_empty_file(int flags, const struct cred *cred)
{
    return __alloc_file(flags, cred);
}

void files_init(void)
{
    filp_cachep =
        kmem_cache_create("filp", sizeof(struct file), 0,
                          SLAB_HWCACHE_ALIGN | SLAB_PANIC | SLAB_ACCOUNT,
                          NULL);
}

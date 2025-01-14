// SPDX-License-Identifier: GPL-2.0

#ifndef _DEVRES_H_
#define _DEVRES_H_

#include <device.h>

typedef void (*dr_release_t)(struct device *dev, void *res);

void *
devres_alloc_node(dr_release_t release, size_t size, gfp_t gfp);

static inline void *
devres_alloc(dr_release_t release, size_t size, gfp_t gfp)
{
    return devres_alloc_node(release, size, gfp);
}

char *
devm_kasprintf(struct device *dev, gfp_t gfp, const char *fmt, ...);

char *
devm_kvasprintf(struct device *dev, gfp_t gfp, const char *fmt, va_list ap);

char *
devm_kstrdup(struct device *dev, const char *s, gfp_t gfp);

void *
devm_ioremap_resource(struct device *dev, const struct resource *res);

void *
devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);

static inline void *
devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return devm_kmalloc(dev, size, gfp | __GFP_ZERO);
}

void
devres_add(struct device *dev, void *res);

void
devres_free(void *res);

#endif /* _DEVRES_H_ */

// SPDX-License-Identifier: GPL-2.0

#include <slab.h>
#include <errno.h>
#include <export.h>
#include <kdev_t.h>
#include <kobj_map.h>

struct kobj_map {
    struct probe {
        struct probe *next;
        dev_t dev;
        unsigned long range;
        kobj_probe_t *get;
        void *data;
    } *probes[255];
};

struct kobj_map *
kobj_map_init(kobj_probe_t *base_probe)
{
    struct kobj_map *p = kmalloc(sizeof(struct kobj_map), GFP_KERNEL);
    struct probe *base = kzalloc(sizeof(*base), GFP_KERNEL);
    int i;

    if ((p == NULL) || (base == NULL)) {
        kfree(p);
        kfree(base);
        return NULL;
    }

    base->dev = 1;
    base->range = ~0;
    base->get = base_probe;
    for (i = 0; i < 255; i++)
        p->probes[i] = base;
    return p;
}
EXPORT_SYMBOL(kobj_map_init);

int
kobj_map(struct kobj_map *domain,
         dev_t dev,
         unsigned long range,
         kobj_probe_t *probe,
         void *data)
{
    unsigned i;
    struct probe *p;
    unsigned index = MAJOR(dev);
    unsigned n = MAJOR(dev + range - 1) - MAJOR(dev) + 1;

    printk("### %s: n(%x) index(%u)\n", __func__, n, index);
    if (n > 255)
        n = 255;

    p = kmalloc_array(n, sizeof(struct probe), GFP_KERNEL);
    if (p == NULL)
        return -ENOMEM;

    for (i = 0; i < n; i++, p++) {
        p->get = probe;
        p->dev = dev;
        p->range = range;
        p->data = data;
    }

    for (i = 0, p -= n; i < n; i++, p++, index++) {
        struct probe **s = &domain->probes[index % 255];
        while (*s && (*s)->range < range)
            s = &(*s)->next;
        p->next = *s;
        *s = p;
    }
    return 0;
}
EXPORT_SYMBOL(kobj_map);

struct kobject *
kobj_lookup(struct kobj_map *domain, dev_t dev, int *index)
{
    struct probe *p;
    struct kobject *kobj;
    unsigned long best = ~0UL;

 retry:
    for (p = domain->probes[MAJOR(dev) % 255]; p; p = p->next) {
        void *data;
        struct kobject *(*probe)(dev_t, int *, void *);

        if (p->dev > dev || p->dev + p->range - 1 < dev)
            continue;
        if (p->range - 1 >= best)
            break;
        data = p->data;
        probe = p->get;
        best = p->range - 1;
        *index = dev - p->dev;
        kobj = probe(dev, index, data);
        if (kobj)
            return kobj;
        goto retry;
    }
    return NULL;
}
EXPORT_SYMBOL(kobj_lookup);

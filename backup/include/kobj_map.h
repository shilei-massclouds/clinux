/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _KOBJ_MAP_H_
#define _KOBJ_MAP_H_

typedef struct kobject *kobj_probe_t(dev_t, int *, void *);

struct kobj_map *
kobj_map_init(kobj_probe_t *);

int
kobj_map(struct kobj_map *domain,
         dev_t dev,
         unsigned long range,
         kobj_probe_t *probe,
         void *data);

struct kobject *
kobj_lookup(struct kobj_map *domain, dev_t dev, int *index);

#endif /* _KOBJ_MAP_H_ */

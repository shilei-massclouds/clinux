// SPDX-License-Identifier: GPL-2.0
#ifndef _KOBJECT_H_
#define _KOBJECT_H_

#include <acgcc.h>
#include <types.h>
#include <list.h>
#include <kref.h>
#include <uidgid.h>

struct kobject {
    const char *name;

    struct list_head entry;

    struct kset *kset;
};

struct kset {
    struct list_head list;
    struct kobject kobj;
};

struct kobj_type {
    void (*release)(struct kobject *kobj);
    const struct sysfs_ops *sysfs_ops;
    struct attribute **default_attrs;   /* use default_groups instead */
    const struct attribute_group **default_groups;
    const struct kobj_ns_type_operations *(*child_ns_type)(struct kobject *kobj);
    const void *(*namespace)(struct kobject *kobj);
    void (*get_ownership)(struct kobject *kobj, kuid_t *uid, kgid_t *gid);
};

void kobject_init(struct kobject *kobj);
extern void kobject_put(struct kobject *kobj);

static inline const char *
kobject_name(const struct kobject *kobj)
{
    return kobj->name;
}

int
kobject_set_name_vargs(struct kobject *kobj, const char *fmt, va_list vargs);

struct kobject *
kset_find_obj(struct kset *kset, const char *name);

struct kset *
kset_create_and_add(const char *name);

int
kobject_init_and_add(struct kobject *kobj, const char *fmt, ...);

struct kobject *
kobject_get_unless_zero(struct kobject *kobj);

#endif /* _KOBJECT_H_ */

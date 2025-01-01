/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _SYSFS_H_
#define _SYSFS_H_

#include <kobject.h>

struct attribute {
    const char  *name;
    umode_t     mode;
};

struct bin_attribute {
    struct attribute    attr;
    size_t              size;
    void                *private;
    /*
    ssize_t (*read)(struct file *,
                    struct kobject *,
                    struct bin_attribute *,
                    char *, loff_t, size_t);

    ssize_t (*write)(struct file *,
                     struct kobject *,
                     struct bin_attribute *,
                     char *, loff_t, size_t);

    int (*mmap)(struct file *,
                struct kobject *,
                struct bin_attribute *attr,
                struct vm_area_struct *vma);
    */
};

struct attribute_group {
    const char      *name;
    umode_t         (*is_visible)(struct kobject *,
                          struct attribute *, int);
    umode_t         (*is_bin_visible)(struct kobject *,
                          struct bin_attribute *, int);
    struct attribute    **attrs;
    struct bin_attribute    **bin_attrs;
};

struct sysfs_ops {
    ssize_t (*show)(struct kobject *, struct attribute *, char *);
    ssize_t (*store)(struct kobject *, struct attribute *, const char *, size_t);
};

#endif /* _SYSFS_H_ */

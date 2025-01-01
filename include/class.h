// SPDX-License-Identifier: GPL-2.0

#ifndef _DEVICE_CLASS_H_
#define _DEVICE_CLASS_H_

#include <device.h>

struct class {
    const char *name;
    struct subsys_private *p;
};

struct class_dev_iter {
    struct klist_iter           ki;
    const struct device_type    *type;
};

int __class_register(struct class *class);

#define class_register(class)           \
({                                      \
    __class_register(class);            \
})

void
class_dev_iter_init(struct class_dev_iter *iter,
                    struct class *class,
                    struct device *start,
                    const struct device_type *type);

struct device *
class_dev_iter_next(struct class_dev_iter *iter);

void
class_dev_iter_exit(struct class_dev_iter *iter);

#endif /* _DEVICE_CLASS_H_ */

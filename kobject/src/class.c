// SPDX-License-Identifier: GPL-2.0

#include <slab.h>
#include <class.h>
#include <errno.h>
#include <klist.h>
#include <export.h>

int
__class_register(struct class *cls)
{
    struct subsys_private *cp;

    cp = kzalloc(sizeof(*cp), GFP_KERNEL);
    if (!cp)
        return -ENOMEM;

    klist_init(&cp->klist_devices);
    cp->class = cls;
    cls->p = cp;
    return 0;
}
EXPORT_SYMBOL(__class_register);

static struct device *
klist_class_to_dev(struct klist_node *n)
{
    struct device_private *p = to_device_private_class(n);
    return p->device;
}

void
class_dev_iter_init(struct class_dev_iter *iter,
                    struct class *class,
                    struct device *start,
                    const struct device_type *type)
{
    struct klist_node *start_knode = NULL;

    if (start)
        start_knode = &start->p->knode_class;

    klist_iter_init_node(&class->p->klist_devices, &iter->ki, start_knode);
    iter->type = type;
}
EXPORT_SYMBOL(class_dev_iter_init);

struct device *
class_dev_iter_next(struct class_dev_iter *iter)
{
    struct klist_node *knode;
    struct device *dev;

    while (1) {
        knode = klist_next(&iter->ki);
        if (!knode)
            return NULL;
        dev = klist_class_to_dev(knode);
        if (!iter->type || iter->type == dev->type)
            return dev;
    }
}
EXPORT_SYMBOL(class_dev_iter_next);

void
class_dev_iter_exit(struct class_dev_iter *iter)
{
    klist_iter_exit(&iter->ki);
}
EXPORT_SYMBOL(class_dev_iter_exit);


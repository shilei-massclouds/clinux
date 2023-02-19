// SPDX-License-Identifier: GPL-2.0+

#include <bug.h>
#include <slab.h>
#include <class.h>
#include <errno.h>
#include <genhd.h>
#include <major.h>
#include <blkdev.h>
#include <export.h>
#include <kdev_t.h>
#include <string.h>
#include <elevator.h>
#include <kobj_map.h>

#define BLKDEV_MAJOR_HASH_SIZE 255
static struct blk_major_name {
    struct blk_major_name *next;
    int major;
    char name[16];
} *major_names[BLKDEV_MAJOR_HASH_SIZE];

struct class block_class = {
    .name = "block",
};

const struct device_type disk_type = {
    .name = "disk",
};

static struct kobj_map *bdev_map;

int
disk_expand_part_tbl(struct gendisk *disk, int partno)
{
    int target;
    struct disk_part_tbl *ptbl;

    target = partno + 1;
    if (target < 0)
        return -EINVAL;

    ptbl = kzalloc_node(struct_size(ptbl, part, target), GFP_KERNEL);
    if (!ptbl)
        return -ENOMEM;

    ptbl->len = target;
    disk->part_tbl = ptbl;
    return 0;
}

struct gendisk *
__alloc_disk_node(int minors)
{
    struct gendisk *disk;
    struct disk_part_tbl *ptbl;

    if (minors > DISK_MAX_PARTS) {
        panic("block: can't allocate more than %d partitions\n",
              DISK_MAX_PARTS);
    }

    disk = kzalloc_node(sizeof(struct gendisk), GFP_KERNEL);
    if (disk) {
        if (disk_expand_part_tbl(disk, 0))
            panic("bad expand tbl!");

        ptbl = disk->part_tbl;
        ptbl->part[0] = &disk->part0;

        disk->minors = minors;
        disk_to_dev(disk)->class = &block_class;
        disk_to_dev(disk)->type = &disk_type;
        device_initialize(disk_to_dev(disk));
    }
    return disk;
}
EXPORT_SYMBOL(__alloc_disk_node);

static inline int
major_to_index(unsigned major)
{
    return major % BLKDEV_MAJOR_HASH_SIZE;
}

static void
register_disk(struct device *parent,
              struct gendisk *disk,
              const struct attribute_group **groups)
{
    struct device *ddev = disk_to_dev(disk);

    ddev->parent = parent;

    dev_set_name(ddev, "%s", disk->disk_name);

    if (device_add(ddev))
        return;

    /* Todo: */
}

int
blk_alloc_devt(struct hd_struct *part, dev_t *devt)
{
    struct gendisk *disk = part_to_disk(part);

    if (part->partno < disk->minors) {
        *devt = MKDEV(disk->major, disk->first_minor + part->partno);
        return 0;
    }

    panic("bad partno (%d)", part->partno);
}

/*
 * Register device numbers dev..(dev+range-1)
 * range must be nonzero
 * The hash chain is sorted on range, so that subranges can override.
 */
void
blk_register_region(dev_t devt,
                    unsigned long range,
                    struct kobject *(*probe)(dev_t, int *, void *),
                    void *data)
{
    kobj_map(bdev_map, devt, range, probe, data);
}

static struct kobject *
exact_match(dev_t devt, int *partno, void *data)
{
    struct gendisk *p = data;
    return &disk_to_dev(p)->kobj;
}

static void
__device_add_disk(struct device *parent,
                  struct gendisk *disk,
                  const struct attribute_group **groups,
                  bool register_queue)
{
    dev_t devt;
    struct device *dev = disk_to_dev(disk);

    BUG_ON(blk_alloc_devt(&disk->part0, &devt));

    if (register_queue)
        elevator_init_mq(disk->queue);

    disk->major = MAJOR(devt);
    disk->first_minor = MINOR(devt);
    dev->devt = devt;

    blk_register_region(disk_devt(disk), disk->minors, exact_match, disk);

    printk("%s: (%x) ma(%x) mi(%x)\n",
           __func__, dev->devt, disk->major, disk->first_minor);

    register_disk(parent, disk, groups);
}

void
device_add_disk(struct device *parent,
                struct gendisk *disk,
                const struct attribute_group **groups)
{
    __device_add_disk(parent, disk, groups, true);
}
EXPORT_SYMBOL(device_add_disk);

int
register_blkdev(unsigned int major, const char *name)
{
    int index;
    struct blk_major_name *p;
    struct blk_major_name **n;
    int ret = 0;

    /* temporary */
    if (major == 0) {
        for (index = ARRAY_SIZE(major_names)-1; index > 0; index--) {
            if (major_names[index] == NULL)
                break;
        }

        if (index == 0)
            panic("%s: failed to get major for %s\n", __func__, name);

        major = index;
        ret = major;
    }

    if (major >= BLKDEV_MAJOR_MAX)
        panic("%s: major requested (%u) is greater than the max(%u) for %s",
              __func__, major, BLKDEV_MAJOR_MAX-1, name);

    p = kmalloc(sizeof(struct blk_major_name), GFP_KERNEL);
    if (p == NULL)
        panic("NO memory!");

    p->major = major;
    strlcpy(p->name, name, sizeof(p->name));
    p->next = NULL;
    index = major_to_index(major);

    for (n = &major_names[index]; *n; n = &(*n)->next) {
        if ((*n)->major == major)
            break;
    }
    if (!*n)
        *n = p;
    else
        panic("BUSY!");

    if (ret < 0)
        panic("register_blkdev: cannot get major %u for %s\n",
              major, name);

    return ret;
}
EXPORT_SYMBOL(register_blkdev);

dev_t
blk_lookup_devt(const char *name, int partno)
{
    dev_t devt = MKDEV(0, 0);
    struct class_dev_iter iter;
    struct device *dev;

    class_dev_iter_init(&iter, &block_class, NULL, &disk_type);
    while ((dev = class_dev_iter_next(&iter))) {
        struct gendisk *disk = dev_to_disk(dev);
        struct hd_struct *part;

        if (strcmp(dev_name(dev), name))
            continue;

        if (partno < disk->minors) {
            /* We need to return the right devno, even
             * if the partition doesn't exist yet.
             */
            printk("%s: %s devt(%x)\n", __func__, dev_name(dev), dev->devt);
            devt = MKDEV(MAJOR(dev->devt), MINOR(dev->devt) + partno);
            break;
        }
        panic("%s: bad partno(%d)", __func__, partno);
    }
    class_dev_iter_exit(&iter);
    return devt;
}
EXPORT_SYMBOL(blk_lookup_devt);

struct gendisk *
get_gendisk(dev_t devt, int *partno)
{
    struct kobject *kobj;
    struct gendisk *disk = NULL;

    BUG_ON(MAJOR(devt) == BLOCK_EXT_MAJOR);

    kobj = kobj_lookup(bdev_map, devt, partno);
    BUG_ON(!kobj);
    if (kobj)
        disk = dev_to_disk(kobj_to_dev(kobj));

    return disk;
}
EXPORT_SYMBOL(get_gendisk);

static struct kobject *
base_probe(dev_t devt, int *partno, void *data)
{
    return NULL;
}

struct hd_struct *
__disk_get_part(struct gendisk *disk, int partno)
{
    struct disk_part_tbl *ptbl = disk->part_tbl;
    BUG_ON(!ptbl);

    if (unlikely(partno < 0 || partno >= ptbl->len))
        return NULL;
    return ptbl->part[partno];
}

struct hd_struct *
disk_get_part(struct gendisk *disk, int partno)
{
    struct hd_struct *part;

    part = __disk_get_part(disk, partno);
    if (part)
        get_device(part_to_dev(part));

    return part;
}
EXPORT_SYMBOL(disk_get_part);

int
init_module(void)
{
    printk("module[genhd]: init begin ...\n");
    BUG_ON(!slab_is_available());
    BUG_ON(class_register(&block_class));
    bdev_map = kobj_map_init(base_probe);
    printk("module[genhd]: init end!\n");
    return 0;
}

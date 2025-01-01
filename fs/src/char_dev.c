// SPDX-License-Identifier: GPL-2.0

#include <fs.h>
#include <slab.h>
#include <device.h>
#include <export.h>
#include <kdev_t.h>
#include <kobject.h>
#include <kobj_map.h>

static struct kobj_map *cdev_map;

#define CHRDEV_MAJOR_HASH_SIZE 255

static struct char_device_struct {
    struct char_device_struct *next;
    unsigned int major;
    unsigned int baseminor;
    int minorct;
    char name[64];
    struct cdev *cdev;      /* will die */
} *chrdevs[CHRDEV_MAJOR_HASH_SIZE];

static struct kobject *exact_match(dev_t dev, int *part, void *data)
{
    struct cdev *p = data;
    return &p->kobj;
}

/**
 * cdev_add() - add a char device to the system
 * @p: the cdev structure for the device
 * @dev: the first device number for which this device is responsible
 * @count: the number of consecutive minor numbers corresponding to this
 *         device
 *
 * cdev_add() adds the device represented by @p to the system, making it
 * live immediately.  A negative error code is returned on failure.
 */
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
{
    int error;

    printk("%s: dev(%lx) count(%u)\n", __func__, dev, count);

    p->dev = dev;
    p->count = count;

    if (dev == WHITEOUT_DEV)
        return -EBUSY;

    error = kobj_map(cdev_map, dev, count, exact_match, p);
    if (error)
        return error;

    return 0;
}
EXPORT_SYMBOL(cdev_add);

static struct kobject *cdev_get(struct cdev *p)
{
    struct kobject *kobj;

    kobj = kobject_get_unless_zero(&p->kobj);
    return kobj;
}

static int chrdev_open(struct inode *inode, struct file *filp)
{
    struct cdev *p;
    const struct file_operations *fops;
    int ret = 0;
    struct cdev *new = NULL;

    p = inode->i_cdev;
    if (!p) {
        int idx;
        struct kobject *kobj;
        kobj = kobj_lookup(cdev_map, inode->i_rdev, &idx);
        if (!kobj) {
            panic("no i_rdev(%lx)", inode->i_rdev);
            return -ENXIO;
        }
        new = container_of(kobj, struct cdev, kobj);
        /* Check i_cdev again in case somebody beat us to it while
           we dropped the lock. */
        p = inode->i_cdev;
        if (!p) {
            inode->i_cdev = p = new;
            list_add(&inode->i_devices, &p->list);
            new = NULL;
        } else if (!cdev_get(p))
            ret = -ENXIO;

        printk("%s: i_cdev(%lx)\n", __func__, inode->i_cdev);
    } else if (!cdev_get(p))
        ret = -ENXIO;

    if (ret)
        return ret;

    ret = -ENXIO;
    fops = fops_get(p->ops);
    if (!fops)
        panic("no fops!");

    replace_fops(filp, fops);
    if (filp->f_op->open) {
        printk("%s: open ...\n", __func__);
        ret = filp->f_op->open(inode, filp);
        if (ret)
            panic("no open!");
    }
    return 0;
}

/*
 * Dummy default file-operations: the only thing this does
 * is contain the open that then fills in the correct operations
 * depending on the special file...
 */
const struct file_operations def_chr_fops = {
    .open = chrdev_open,
};
EXPORT_SYMBOL(def_chr_fops);

static struct kobject *base_probe(dev_t dev, int *part, void *data)
{
    return NULL;
}

/**
 * cdev_alloc() - allocate a cdev structure
 *
 * Allocates and returns a cdev structure, or NULL on failure.
 */
struct cdev *cdev_alloc(void)
{
    struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
    if (p) {
        INIT_LIST_HEAD(&p->list);
        kobject_init(&p->kobj);
    }
    return p;
}
EXPORT_SYMBOL(cdev_alloc);

void chrdev_init(void)
{
    cdev_map = kobj_map_init(base_probe);
}

/**
 * cdev_init() - initialize a cdev structure
 * @cdev: the structure to initialize
 * @fops: the file_operations for this device
 *
 * Initializes @cdev, remembering @fops, making it ready to add to the
 * system with cdev_add().
 */
void cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
    memset(cdev, 0, sizeof *cdev);
    INIT_LIST_HEAD(&cdev->list);
    kobject_init(&cdev->kobj);
    cdev->ops = fops;
}
EXPORT_SYMBOL(cdev_init);

static struct char_device_struct *
__register_chrdev_region(unsigned int major, unsigned int baseminor,
                         int minorct, const char *name)
{
    return NULL;
}

/**
 * register_chrdev_region() - register a range of device numbers
 * @from: the first in the desired range of device numbers; must include
 *        the major number.
 * @count: the number of consecutive device numbers required
 * @name: the name of the device or driver.
 *
 * Return value is zero on success, a negative error code on failure.
 */
int register_chrdev_region(dev_t from, unsigned count, const char *name)
{
    struct char_device_struct *cd;
    dev_t to = from + count;
    dev_t n, next;

    for (n = from; n < to; n = next) {
        next = MKDEV(MAJOR(n)+1, 0);
        if (next > to)
            next = to;
        cd = __register_chrdev_region(MAJOR(n), MINOR(n),
                                      next - n, name);
        if (IS_ERR(cd))
            panic("register error!");
    }
    return 0;
}
EXPORT_SYMBOL(register_chrdev_region);

/* SPDX-License-Identifier: GPL-2.0 */

#include <tty.h>
#include <cdev.h>
#include <slab.h>
#include <major.h>
#include <export.h>
#include <kdev_t.h>
#include <uaccess.h>
#include <tty_ldisc.h>

static struct cdev console_cdev;
static struct file *redirect;

static struct tty_driver *
tty_lookup_driver(dev_t device, struct file *filp, int *index)
{
    struct tty_driver *driver = NULL;

    switch (device) {
    case MKDEV(TTYAUX_MAJOR, 1): {
        struct tty_driver *console_driver = console_device(index);
        printk("%s: 2\n", __func__);
        if (console_driver) {
            printk("%s: 3\n", __func__);
            driver = console_driver;
            if (driver && filp) {
                /* Don't let /dev/console block */
                filp->f_flags |= O_NONBLOCK;
                break;
            }
        }
        return ERR_PTR(-ENODEV);
    }
    default:
        panic("no driver!");
        break;
    }
    printk("%s: ! %p\n", __func__, driver);
    return driver;
}

/**
 *  tty_driver_lookup_tty() - find an existing tty, if any
 *  @driver: the driver for the tty
 *  @idx:    the minor number
 *
 *  Return the tty, if found. If not found, return NULL or ERR_PTR() if the
 *  driver lookup() method returns an error.
 *
 *  Locking: tty_mutex must be held. If the tty is found, bump the tty kref.
 */
static struct tty_struct *
tty_driver_lookup_tty(struct tty_driver *driver, struct file *file, int idx)
{
    struct tty_struct *tty;

    if (driver->ops->lookup)
        if (!file)
            tty = ERR_PTR(-EIO);
        else
            tty = driver->ops->lookup(driver, file, idx);
    else
        tty = driver->ttys[idx];

    return tty;
}

struct tty_struct *alloc_tty_struct(struct tty_driver *driver, int idx)
{
    struct tty_struct *tty;

    tty = kzalloc(sizeof(*tty), GFP_KERNEL);
    if (!tty)
        return NULL;
    if (tty_ldisc_init(tty))
        panic("init ldisc error!");

    INIT_LIST_HEAD(&tty->tty_files);

    tty->driver = driver;
    tty->ops = driver->ops;
    tty->index = idx;
    return tty;
}

static int
tty_driver_install_tty(struct tty_driver *driver, struct tty_struct *tty)
{
    return driver->ops->install ? driver->ops->install(driver, tty) :
        tty_standard_install(driver, tty);
}

struct tty_struct *tty_init_dev(struct tty_driver *driver, int idx)
{
    int retval;
    struct tty_struct *tty;

    tty = alloc_tty_struct(driver, idx);
    if (!tty)
        panic("Out of memory!");

    retval = tty_driver_install_tty(driver, tty);
    if (retval < 0)
        panic("install tty error!");

    return tty;
}

static struct tty_struct *
tty_open_by_driver(dev_t device, struct file *filp)
{
    struct tty_struct *tty;
    int index = -1;
    struct tty_driver *driver = NULL;

    printk("%s: 0\n", __func__);
    driver = tty_lookup_driver(device, filp, &index);
    if (IS_ERR_OR_NULL(driver))
        panic("bad driver!");

    printk("%s: 1 name(%s,%s) mm(%x,%x)\n",
           __func__, driver->name, driver->driver_name,
           driver->major, driver->minor_start);

    /* check whether we're reopening an existing tty */
    tty = tty_driver_lookup_tty(driver, filp, index);
    if (IS_ERR(tty))
        panic("bad tty!");

    if (tty) {
        panic("find tty!");
    } else {
        tty = tty_init_dev(driver, index);
    }

    printk("%s: device(%d) tty(%p)!\n", __func__, device, tty);
    return tty;
}

/* Associate a new file with the tty structure */
void tty_add_file(struct tty_struct *tty, struct file *file)
{
    struct tty_file_private *priv = file->private_data;

    priv->tty = tty;
    priv->file = file;

    list_add(&priv->list, &tty->tty_files);
}

int tty_alloc_file(struct file *file)
{
    struct tty_file_private *priv;

    priv = kmalloc(sizeof(*priv), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;

    file->private_data = priv;
    return 0;
}

static int tty_open(struct inode *inode, struct file *filp)
{
    int retval;
    struct tty_struct *tty;
    dev_t device = inode->i_rdev;
    unsigned saved_flags = filp->f_flags;

    nonseekable_open(inode, filp);

    retval = tty_alloc_file(filp);
    if (retval)
        panic("Out of memory!");

    printk("%s: dev(%x)\n", __func__, device);
    tty = tty_open_by_driver(device, filp);
    if (IS_ERR_OR_NULL(tty))
        panic("open by driver error!");

    printk("%s: 1 ops(%p)\n", __func__, tty->ops);
    tty_add_file(tty, filp);

    if (tty->ops->open)
        retval = tty->ops->open(tty, filp);
    else
        retval = -ENODEV;
    filp->f_flags = saved_flags;
    printk("%s: 2\n", __func__);

    if (retval)
        panic("open tty error!");

    clear_bit(TTY_HUPPED, &tty->flags);
    printk("%s: device(%x)!\n", __func__, device);
    return 0;
}

static inline struct tty_struct *file_tty(struct file *file)
{
    return ((struct tty_file_private *)file->private_data)->tty;
}

/*
 * Split writes up in sane blocksizes to avoid
 * denial-of-service type attacks
 */
static inline ssize_t
do_tty_write(ssize_t (*write)(struct tty_struct *, struct file *,
                              const unsigned char *, size_t),
             struct tty_struct *tty,
             struct file *file,
             const char *buf,
             size_t count)
{
    ssize_t ret;
    unsigned int chunk;
    ssize_t written = 0;

    chunk = 2048;
    if (test_bit(TTY_NO_WRITE_SPLIT, &tty->flags))
        chunk = 65536;
    if (count < chunk)
        chunk = count;

    /* write_buf/write_cnt is protected by the atomic_write_lock mutex */
    if (tty->write_cnt < chunk) {
        unsigned char *buf_chunk;

        if (chunk < 1024)
            chunk = 1024;

        buf_chunk = kmalloc(chunk, GFP_KERNEL);
        if (!buf_chunk)
            panic("Out of memory!");
        kfree(tty->write_buf);
        tty->write_cnt = chunk;
        tty->write_buf = buf_chunk;
    }

    /* Do the write .. */
    for (;;) {
        size_t size = count;
        if (size > chunk)
            size = chunk;
        ret = -EFAULT;
        if (copy_from_user(tty->write_buf, buf, size))
            break;
        ret = write(tty, file, tty->write_buf, size);
        if (ret <= 0)
            break;
        written += ret;
        buf += ret;
        count -= ret;
        if (!count)
            break;
    }
    if (written) {
        ret = written;
    }
    return ret;
}

static ssize_t
tty_write(struct file *file,
          const char *buf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    struct tty_ldisc *ld;
    struct tty_struct *tty = file_tty(file);

    if (!tty || !tty->ops->write || tty_io_error(tty))
        return -EIO;
    /* Short term debug to catch buggy drivers */
    if (tty->ops->write_room == NULL)
        panic("missing write_room method");
    ld = tty_ldisc_ref_wait(tty);
    if (!ld)
        panic("get ld error!");
    if (!ld->ops->write)
        ret = -EIO;
    else
        ret = do_tty_write(ld->ops->write, tty, file, buf, count);

    return ret;
}

ssize_t
redirected_tty_write(struct file *file,
                     const char *buf, size_t count, loff_t *ppos)
{
    struct file *p = NULL;

    if (redirect)
        p = redirect;

    if (p) {
        ssize_t res;
        res = vfs_write(p, buf, count, &p->f_pos);
        return res;
    }
    return tty_write(file, buf, count, ppos);
}

static const struct file_operations console_fops = {
    .open       = tty_open,
    .write      = redirected_tty_write,
    /*
    .llseek     = no_llseek,
    .read       = tty_read,
    .poll       = tty_poll,
    .unlocked_ioctl = tty_ioctl,
    .compat_ioctl   = tty_compat_ioctl,
    .release    = tty_release,
    .fasync     = tty_fasync,
    */
};

static int
tty_cdev_add(struct tty_driver *driver, dev_t dev,
             unsigned int index, unsigned int count)
{
    int err;

    /* init here, since reused cdevs cause crashes */
    driver->cdevs[index] = cdev_alloc();
    if (!driver->cdevs[index])
        return -ENOMEM;
    //driver->cdevs[index]->ops = &tty_fops;
    err = cdev_add(driver->cdevs[index], dev, count);
    if (err)
        kobject_put(&driver->cdevs[index]->kobj);
    return err;
}

/**
 *  tty_line_name   -   generate name for a tty
 *  @driver: the tty driver in use
 *  @index: the minor number
 *  @p: output buffer of at least 7 bytes
 *
 *  Generate a name from a driver reference and write it to the output
 *  buffer.
 *
 *  Locking: None
 */
static ssize_t
tty_line_name(struct tty_driver *driver, int index, char *p)
{
    if (driver->flags & TTY_DRIVER_UNNUMBERED_NODE)
        return sprintf(p, "%s", driver->name);
    else
        return sprintf(p, "%s%d", driver->name,
                       index + driver->name_base);
}

struct device *
tty_register_device_attr(struct tty_driver *driver,
                         unsigned index,
                         struct device *device,
                         void *drvdata,
                         const struct attribute_group **attr_grp)
{
    int retval;
    char name[64];
    struct device *dev;
    dev_t devt = MKDEV(driver->major, driver->minor_start) + index;

    if (index >= driver->num)
        panic("%s: Attempt to register invalid tty line number (%d)",
              driver->name, index);

    if (driver->type == TTY_DRIVER_TYPE_PTY)
        panic("TTY_DRIVER_TYPE_PTY!");
    else
        tty_line_name(driver, index, name);

    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return ERR_PTR(-ENOMEM);

    dev->devt = devt;
    //dev->class = tty_class;
    dev->parent = device;
    dev_set_name(dev, "%s", name);
    dev_set_drvdata(dev, drvdata);

    printk("%s: devt(%x) (%x,%x)\n",
           __func__, devt, driver->major, driver->minor_start);

    if (!(driver->flags & TTY_DRIVER_DYNAMIC_ALLOC)) {
        retval = tty_cdev_add(driver, devt, index, 1);
        if (retval)
            panic("tty: add cdev error!");
    }
    return dev;
}

void
tty_port_link_device(struct tty_port *port,
                     struct tty_driver *driver, unsigned index)
{
    BUG_ON(index >= driver->num);
    driver->ports[index] = port;
}
EXPORT_SYMBOL(tty_port_link_device);

struct device *
tty_port_register_device_attr_serdev(struct tty_port *port,
                                     struct tty_driver *driver,
                                     unsigned index,
                                     struct device *device,
                                     void *drvdata,
                                     const struct attribute_group **attr_grp)
{
    struct device *dev;

    tty_port_link_device(port, driver, index);

    return tty_register_device_attr(driver, index, device, drvdata,
                                    attr_grp);
}
EXPORT_SYMBOL(tty_port_register_device_attr_serdev);

/**
 * __tty_alloc_driver -- allocate tty driver
 * @lines: count of lines this driver can handle at most
 * @owner: module which is responsible for this driver
 * @flags: some of TTY_DRIVER_* flags, will be set in driver->flags
 *
 * This should not be called directly, some of the provided macros should be
 * used instead. Use IS_ERR and friends on @retval.
 */
struct tty_driver *
__tty_alloc_driver(unsigned int lines, unsigned long flags)
{
    struct tty_driver *driver;
    unsigned int cdevs = 1;

    driver = kzalloc(sizeof(*driver), GFP_KERNEL);
    if (!driver)
        return ERR_PTR(-ENOMEM);

    driver->num = lines;
    driver->flags = flags;

    if (!(flags & TTY_DRIVER_DEVPTS_MEM)) {
        driver->ttys = kcalloc(lines, sizeof(*driver->ttys),
                               GFP_KERNEL);
        if (!driver->ttys)
            panic("Out of memory!");
    }

    if (!(flags & TTY_DRIVER_DYNAMIC_ALLOC)) {
        driver->ports = kcalloc(lines, sizeof(*driver->ports),
                                GFP_KERNEL);
        if (!driver->ports)
            panic("Out of memory!");
        cdevs = lines;
    }

    driver->cdevs = kcalloc(cdevs, sizeof(*driver->cdevs), GFP_KERNEL);
    if (!driver->cdevs)
        panic("Out of memory!");

    return driver;
}
EXPORT_SYMBOL(__tty_alloc_driver);

void
tty_set_operations(struct tty_driver *driver,
                   const struct tty_operations *op)
{
    driver->ops = op;
};
EXPORT_SYMBOL(tty_set_operations);

int tty_init(void)
{
    cdev_init(&console_cdev, &console_fops);
    if (cdev_add(&console_cdev, MKDEV(TTYAUX_MAJOR, 1), 1) ||
        register_chrdev_region(MKDEV(TTYAUX_MAJOR, 1), 1, "/dev/console") < 0)
        panic("Couldn't register /dev/console driver\n");

    return 0;
}

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TTY_H
#define _LINUX_TTY_H

#include <file.h>
#include <sysfs.h>
#include <kernel.h>

#define NR_LDISCS   30

/* line disciplines */
#define N_TTY       0

/*
 * These bits are used in the flags field of the tty structure.
 *
 * So that interrupts won't be able to mess up the queues,
 * copy_to_cooked must be atomic with respect to itself, as must
 * tty->write.  Thus, you must use the inline functions set_bit() and
 * clear_bit() to make things atomic.
 */
#define TTY_THROTTLED       0   /* Call unthrottle() at threshold min */
#define TTY_IO_ERROR        1   /* Cause an I/O error (may be no ldisc too) */
#define TTY_NO_WRITE_SPLIT  17  /* Preserve write boundaries to driver */
#define TTY_HUPPED          18  /* Post driver->hangup() */

/* tty driver types */
#define TTY_DRIVER_TYPE_PTY         0x0004
#define TTY_DRIVER_DEVPTS_MEM       0x0010
#define TTY_DRIVER_DYNAMIC_ALLOC    0x0040
#define TTY_DRIVER_UNNUMBERED_NODE  0x0080

struct tty_struct {
    struct tty_driver *driver;
    const struct tty_operations *ops;
    int index;
    struct tty_ldisc *ldisc;
    unsigned long flags;
    void *driver_data;
    unsigned char *write_buf;
    int write_cnt;
    struct list_head tty_files;
};

/* Each of a tty's open files has private_data pointing to tty_file_private */
struct tty_file_private {
    struct tty_struct *tty;
    struct file *file;
    struct list_head list;
};

struct tty_operations {
    struct tty_struct * (*lookup)(struct tty_driver *driver,
                                  struct file *filp, int idx);
    int (*install)(struct tty_driver *driver, struct tty_struct *tty);
    int (*open)(struct tty_struct * tty, struct file * filp);
    int (*write)(struct tty_struct * tty,
                 const unsigned char *buf, int count);
    int (*write_room)(struct tty_struct *tty);
};

struct tty_port;

struct tty_port_operations {
    int (*activate)(struct tty_port *port, struct tty_struct *tty);
};

struct tty_port {
    struct tty_struct *tty;         /* Back pointer */
    unsigned long   flags;          /* User TTY flags ASYNC_ */
    unsigned long   iflags;         /* Internal flags TTY_PORT_ */

    const struct tty_port_operations *ops;  /* Port operations */
    unsigned char   console:1,      /* port is a console */
                    low_latency:1;  /* optional: tune for latency */
};

struct tty_driver {
    struct cdev **cdevs;
    const char *driver_name;
    const char *name;
    int name_base;      /* offset of printed name */
    int major;          /* major device number */
    int minor_start;    /* start of minor device number */
    unsigned int num;   /* number of devices allocated */
    short type;         /* type of tty driver */
    unsigned long flags;    /* tty driver flags */

    /*
     * Pointer to the tty data structures
     */
    struct tty_struct **ttys;
    struct tty_port **ports;
    void *driver_state;

    /*
     * Driver methods
     */
    const struct tty_operations *ops;
};

struct device *
tty_register_device_attr(struct tty_driver *driver,
                         unsigned index,
                         struct device *device,
                         void *drvdata,
                         const struct attribute_group **attr_grp);

struct device *
tty_port_register_device_attr_serdev(struct tty_port *port,
                                     struct tty_driver *driver,
                                     unsigned index,
                                     struct device *device,
                                     void *drvdata,
                                     const struct attribute_group **attr_grp);

extern struct tty_driver *
__tty_alloc_driver(unsigned int lines, unsigned long flags);

/* Use TTY_DRIVER_* flags below */
#define tty_alloc_driver(lines, flags) __tty_alloc_driver(lines, flags)

/*
 * DEPRECATED Do not use this in new code, use tty_alloc_driver instead.
 * (And change the return value checks.)
 */
static inline struct tty_driver *
alloc_tty_driver(unsigned int lines)
{
    struct tty_driver *ret = tty_alloc_driver(lines, 0);
    if (IS_ERR(ret))
        return NULL;
    return ret;
}

void
tty_port_link_device(struct tty_port *port,
                     struct tty_driver *driver, unsigned index);

void
tty_set_operations(struct tty_driver *driver,
                   const struct tty_operations *op);

int
tty_standard_install(struct tty_driver *driver, struct tty_struct *tty);

/* tty_port::iflags bits -- use atomic bit ops */
#define TTY_PORT_INITIALIZED    0   /* device is initialized */
#define TTY_PORT_SUSPENDED      1   /* device is suspended */
#define TTY_PORT_ACTIVE         2   /* device is open */

static inline bool tty_port_initialized(struct tty_port *port)
{
    return test_bit(TTY_PORT_INITIALIZED, &port->iflags);
}

int
tty_port_open(struct tty_port *port,
              struct tty_struct *tty,
              struct file *filp);

void tty_port_init(struct tty_port *port);

static inline void
tty_port_set_initialized(struct tty_port *port, bool val)
{
    if (val)
        set_bit(TTY_PORT_INITIALIZED, &port->iflags);
    else
        clear_bit(TTY_PORT_INITIALIZED, &port->iflags);
}

static inline bool tty_io_error(struct tty_struct *tty)
{
    return test_bit(TTY_IO_ERROR, &tty->flags);
}

static inline void tty_port_set_active(struct tty_port *port, bool val)
{
    if (val)
        set_bit(TTY_PORT_ACTIVE, &port->iflags);
    else
        clear_bit(TTY_PORT_ACTIVE, &port->iflags);
}

#define O_OPOST(tty)    _O_FLAG((tty), OPOST)

#endif /* _LINUX_TTY_H */

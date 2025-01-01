/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TTY_LDISC_H
#define _LINUX_TTY_LDISC_H

#include <tty.h>

struct tty_ldisc_ops {
    int num;

    ssize_t (*write)(struct tty_struct *tty, struct file *file,
                     const unsigned char *buf, size_t nr);
};

struct tty_ldisc {
    struct tty_ldisc_ops *ops;
    struct tty_struct *tty;
};

int tty_ldisc_init(struct tty_struct *tty);

struct tty_ldisc *tty_ldisc_ref_wait(struct tty_struct *tty);

int tty_register_ldisc(int disc, struct tty_ldisc_ops *new_ldisc);

#endif /* _LINUX_TTY_LDISC_H */

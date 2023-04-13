// SPDX-License-Identifier: GPL-1.0+

#include <tty_ldisc.h>

static ssize_t
n_tty_write(struct tty_struct *tty, struct file *file,
            const unsigned char *buf, size_t nr)
{
    int c;
    ssize_t retval = 0;
    const unsigned char *b = buf;

    while (1) {
        while (nr > 0) {
            c = tty->ops->write(tty, b, nr);
            if (c < 0)
                panic("write error!");

            if (!c)
                break;

            b += c;
            nr -= c;
        }
        if (!nr)
            break;
    }
    return (b - buf) ? b - buf : retval;
}

static struct tty_ldisc_ops n_tty_ops = {
    .write  = n_tty_write,
};

void n_tty_init(void)
{
    tty_register_ldisc(N_TTY, &n_tty_ops);
}

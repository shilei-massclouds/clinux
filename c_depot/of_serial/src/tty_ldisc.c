// SPDX-License-Identifier: GPL-2.0

#include <slab.h>
#include <export.h>
#include <tty_ldisc.h>

/* Line disc dispatch table */
static struct tty_ldisc_ops *tty_ldiscs[NR_LDISCS];

struct tty_ldisc *tty_ldisc_ref_wait(struct tty_struct *tty)
{
    struct tty_ldisc *ld;

    ld = tty->ldisc;
    return ld;
}
EXPORT_SYMBOL(tty_ldisc_ref_wait);

static struct tty_ldisc_ops *get_ldops(int disc)
{
    unsigned long flags;
    struct tty_ldisc_ops *ldops, *ret;

    printk("%s: disc(%d)\n", __func__, disc);
    ret = ERR_PTR(-EINVAL);
    ldops = tty_ldiscs[disc];
    if (ldops)
        ret = ldops;
    else
        panic("no ld ops!");
    return ret;
}

static struct tty_ldisc *tty_ldisc_get(struct tty_struct *tty, int disc)
{
    struct tty_ldisc *ld;
    struct tty_ldisc_ops *ldops;

    if (disc < N_TTY || disc >= NR_LDISCS)
        return ERR_PTR(-EINVAL);

    /*
     * Get the ldisc ops - we may need to request them to be loaded
     * dynamically and try again.
     */
    ldops = get_ldops(disc);
    if (IS_ERR(ldops))
        panic("bad ld ops!");

    /*
     * There is no way to handle allocation failure of only 16 bytes.
     * Let's simplify error handling and save more memory.
     */
    ld = kmalloc(sizeof(struct tty_ldisc), GFP_KERNEL | __GFP_NOFAIL);
    ld->ops = ldops;
    ld->tty = tty;
    return ld;
}

/**
 *  tty_ldisc_init      -   ldisc setup for new tty
 *  @tty: tty being allocated
 *
 *  Set up the line discipline objects for a newly allocated tty. Note that
 *  the tty structure is not completely set up when this call is made.
 */
int tty_ldisc_init(struct tty_struct *tty)
{
    struct tty_ldisc *ld = tty_ldisc_get(tty, N_TTY);
    if (IS_ERR(ld))
        panic("bad ldisc!");
    tty->ldisc = ld;
    return 0;
}
EXPORT_SYMBOL(tty_ldisc_init);

int tty_register_ldisc(int disc, struct tty_ldisc_ops *new_ldisc)
{
    unsigned long flags;
    int ret = 0;

    if (disc < N_TTY || disc >= NR_LDISCS)
        return -EINVAL;

    printk("%s: disc(%d) ldisc(%p)\n", __func__, disc, new_ldisc);
    tty_ldiscs[disc] = new_ldisc;
    new_ldisc->num = disc;
    return ret;
}
EXPORT_SYMBOL(tty_register_ldisc);

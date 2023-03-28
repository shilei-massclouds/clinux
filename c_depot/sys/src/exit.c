// SPDX-License-Identifier: GPL-2.0-only
/*
 *  linux/kernel/exit.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <syscalls.h>

void do_exit(long code)
{
    panic("NOW user-space app exit! [%d]", code);
}

/*
 * Take down every thread in the group.  This is called by fatal signals
 * as well as by sys_exit_group (below).
 */
void
_do_group_exit(int exit_code)
{
    struct signal_struct *sig = current->signal;

    printk("%s: ...\n", __func__);

#if 0
    if (sig->flags & SIGNAL_GROUP_EXIT)
        exit_code = sig->group_exit_code;
    else if (sig->group_exec_task)
        exit_code = 0;
    else if (!thread_group_empty(current)) {
        struct sighand_struct *const sighand = current->sighand;

        spin_lock_irq(&sighand->siglock);
        if (sig->flags & SIGNAL_GROUP_EXIT)
            /* Another thread got here before we took the lock.  */
            exit_code = sig->group_exit_code;
        else if (sig->group_exec_task)
            exit_code = 0;
        else {
            sig->group_exit_code = exit_code;
            sig->flags = SIGNAL_GROUP_EXIT;
            zap_other_threads(current);
        }
        spin_unlock_irq(&sighand->siglock);
    }
#endif

    do_exit(exit_code);
    /* NOTREACHED */
}

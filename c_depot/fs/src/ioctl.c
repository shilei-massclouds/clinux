// SPDX-License-Identifier: GPL-2.0
/*
 *  linux/fs/ioctl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <file.h>
#include <stat.h>
#include <export.h>
#include <syscalls.h>
#include <ioctls.h>
#include <fiemap.h>

static int file_ioctl(struct file *filp, unsigned int cmd, int *p)
{
    panic("%s: cmd (%u)\n", __func__, cmd);
#if 0
    switch (cmd) {
    case FIBMAP:
        return ioctl_fibmap(filp, p);
    case FS_IOC_RESVSP:
    case FS_IOC_RESVSP64:
        return ioctl_preallocate(filp, 0, p);
    case FS_IOC_UNRESVSP:
    case FS_IOC_UNRESVSP64:
        return ioctl_preallocate(filp, FALLOC_FL_PUNCH_HOLE, p);
    case FS_IOC_ZERO_RANGE:
        return ioctl_preallocate(filp, FALLOC_FL_ZERO_RANGE, p);
    }

    return -ENOIOCTLCMD;
#endif
}

/*
 * do_vfs_ioctl() is not for drivers and not intended to be EXPORT_SYMBOL()'d.
 * It's just a simple helper for sys_ioctl and compat_sys_ioctl.
 *
 * When you add any new common ioctls to the switches above and below,
 * please ensure they have compatible arguments in compat mode.
 */
static int
do_vfs_ioctl(struct file *filp, unsigned int fd,
             unsigned int cmd, unsigned long arg)
{
    void *argp = (void *)arg;
    struct inode *inode = file_inode(filp);

    switch (cmd) {
    case FIOCLEX:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //set_close_on_exec(fd, 1);
        return 0;

    case FIONCLEX:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //set_close_on_exec(fd, 0);
        return 0;

    case FIONBIO:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_fionbio(filp, argp);

    case FIOASYNC:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_fioasync(fd, filp, argp);

    case FIOQSIZE:
#if 0
        if (S_ISDIR(inode->i_mode) || S_ISREG(inode->i_mode) ||
            S_ISLNK(inode->i_mode)) {
            loff_t res = inode_get_bytes(inode);
            return copy_to_user(argp, &res, sizeof(res)) ?
                        -EFAULT : 0;
        }

#endif
        panic("%s: cmd (%u)\n", __func__, cmd);
        return -ENOTTY;

    case FIFREEZE:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_fsfreeze(filp);

    case FITHAW:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_fsthaw(filp);

    case FS_IOC_FIEMAP:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_fiemap(filp, argp);

    case FIGETBSZ:
        panic("%s: cmd (%u)\n", __func__, cmd);
        /* anon_bdev filesystems may not have a block size */
#if 0
        if (!inode->i_sb->s_blocksize)
            return -EINVAL;

        return put_user(inode->i_sb->s_blocksize, (int __user *)argp);
#endif

    case FICLONE:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_file_clone(filp, arg, 0, 0, 0);

    case FICLONERANGE:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_file_clone_range(filp, argp);

    case FIDEDUPERANGE:
        panic("%s: cmd (%u)\n", __func__, cmd);
        //return ioctl_file_dedupe_range(filp, argp);

    case FIONREAD:
        panic("%s: cmd (%u)\n", __func__, cmd);
#if 0
        if (!S_ISREG(inode->i_mode))
            return vfs_ioctl(filp, cmd, arg);

        return put_user(i_size_read(inode) - filp->f_pos,
                (int __user *)argp);
#endif

    default:
        if (S_ISREG(inode->i_mode))
            return file_ioctl(filp, cmd, argp);
        break;
    }

    return -ENOIOCTLCMD;
}

/**
 * vfs_ioctl - call filesystem specific ioctl methods
 * @filp:   open file to invoke ioctl method on
 * @cmd:    ioctl command to execute
 * @arg:    command-specific argument for ioctl
 *
 * Invokes filesystem specific ->unlocked_ioctl, if one exists; otherwise
 * returns -ENOTTY.
 *
 * Returns 0 on success, -errno on error.
 */
long vfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int error = -ENOTTY;

    if (!filp->f_op->unlocked_ioctl)
        goto out;

    printk("%s: ...\n", __func__);
    error = filp->f_op->unlocked_ioctl(filp, cmd, arg);
    if (error == -ENOIOCTLCMD)
        error = -ENOTTY;
 out:
    return error;
}
EXPORT_SYMBOL(vfs_ioctl);

long
_ksys_do_vfs_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    struct fd f = fdget(fd);
    int error;

    if (!f.file)
        return -EBADF;

    error = do_vfs_ioctl(f.file, fd, cmd, arg);
    if (error == -ENOIOCTLCMD) {
        error = vfs_ioctl(f.file, cmd, arg);
    }

out:
    fdput(f);
    return error;
}

void
init_ioctl(void)
{
    ksys_do_vfs_ioctl = _ksys_do_vfs_ioctl;
}

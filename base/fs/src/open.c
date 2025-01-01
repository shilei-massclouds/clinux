// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <file.h>
#include <stat.h>
#include <errno.h>
#include <namei.h>
#include <fcntl.h>
#include <dcache.h>
#include <export.h>
#include <fdtable.h>
#include <openat2.h>
#include <syscalls.h>

static int do_dentry_open(struct file *f,
                          struct inode *inode,
                          int (*open)(struct inode *, struct file *))
{
    int error;

    f->f_inode = inode;
    f->f_mapping = inode->i_mapping;

    f->f_op = inode->i_fop;
    BUG_ON(!f->f_op);

    /* POSIX.1-2008/SUSv4 Section XSI 2.9.7 */
    if (S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode))
        f->f_mode |= FMODE_ATOMIC_POS;

    /* normally all 3 are set; ->open() can clear them if needed */
    f->f_mode |= FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE;
    printk("%s 1 f_mode(%x)\n", __func__, f->f_mode);
    if (!open)
        open = f->f_op->open;
    if (open) {
        error = open(inode, f);
        if (error)
            panic("open error!");
    }
    f->f_mode |= FMODE_OPENED;
    if ((f->f_mode & FMODE_READ) &&
        likely(f->f_op->read || f->f_op->read_iter))
        f->f_mode |= FMODE_CAN_READ;
    if ((f->f_mode & FMODE_WRITE) &&
        likely(f->f_op->write || f->f_op->write_iter))
        f->f_mode |= FMODE_CAN_WRITE;

    f->f_flags &= ~(O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC);
    file_ra_state_init(&f->f_ra, f->f_mapping->host->i_mapping);
    printk("########### %s !\n", __func__);
    return 0;
}

/**
 * vfs_open - open the file at the given path
 * @path: path to open
 * @file: newly allocated file with f_flag initialized
 * @cred: credentials to use
 */
int vfs_open(const struct path *path, struct file *file)
{
    file->f_path = *path;
    return do_dentry_open(file, d_backing_inode(path->dentry), NULL);
}

/*
 * Called when an inode is about to be open.
 * We use this to disallow opening large files on 32bit systems if
 * the caller didn't specify O_LARGEFILE.  On 64bit systems we force
 * on this flag in sys_open.
 */
int generic_file_open(struct inode *inode, struct file *filp)
{
    if (!(filp->f_flags & O_LARGEFILE) &&
        i_size_read(inode) > MAX_NON_LFS)
        return -EOVERFLOW;
    return 0;
}
EXPORT_SYMBOL(generic_file_open);

struct filename *
getname(const char *filename)
{
    return getname_flags(filename, 0, NULL);
}

static inline void __clear_open_fd(unsigned int fd, struct fdtable *fdt)
{
    __clear_bit(fd, fdt->open_fds);
    __clear_bit(fd / BITS_PER_LONG, fdt->full_fds_bits);
}

static void __put_unused_fd(struct files_struct *files, unsigned int fd)
{
    struct fdtable *fdt = files_fdtable(files);
    __clear_open_fd(fd, fdt);
    if (fd < files->next_fd)
        files->next_fd = fd;
}

void put_unused_fd(unsigned int fd)
{
    struct files_struct *files = current->files;
    __put_unused_fd(files, fd);
}
EXPORT_SYMBOL(put_unused_fd);

#define WILL_CREATE(flags)  (flags & (O_CREAT | __O_TMPFILE))
#define O_PATH_FLAGS        (O_DIRECTORY | O_NOFOLLOW | O_PATH | O_CLOEXEC)

static int
build_open_flags(const struct open_how *how, struct open_flags *op)
{
    int lookup_flags = 0;
    int flags = how->flags;
    int acc_mode = ACC_MODE(flags);

    /* Must never be set by userspace */
    flags &= ~(FMODE_NONOTIFY | O_CLOEXEC);

    /*
     * Older syscalls implicitly clear all of the invalid flags or argument
     * values before calling build_open_flags(), but openat2(2) checks all
     * of its arguments.
     */
    if (flags & ~VALID_OPEN_FLAGS)
        return -EINVAL;
    if (how->resolve & ~VALID_RESOLVE_FLAGS)
        return -EINVAL;

    /* Deal with the mode. */
    if (WILL_CREATE(flags)) {
        if (how->mode & ~S_IALLUGO)
            return -EINVAL;
        op->mode = how->mode | S_IFREG;
    } else {
        if (how->mode != 0)
            return -EINVAL;
        op->mode = 0;
    }

    /*
     * In order to ensure programs get explicit errors when trying to use
     * O_TMPFILE on old kernels, O_TMPFILE is implemented such that it
     * looks like (O_DIRECTORY|O_RDWR & ~O_CREAT) to old kernels. But we
     * have to require userspace to explicitly set it.
     */
    if (flags & __O_TMPFILE) {
        if ((flags & O_TMPFILE_MASK) != O_TMPFILE)
            return -EINVAL;
        if (!(acc_mode & MAY_WRITE))
            return -EINVAL;
    }
    if (flags & O_PATH) {
        /* O_PATH only permits certain other flags to be set. */
        if (flags & ~O_PATH_FLAGS)
            return -EINVAL;
        acc_mode = 0;
    }

    /*
     * O_SYNC is implemented as __O_SYNC|O_DSYNC.  As many places only
     * check for O_DSYNC if the need any syncing at all we enforce it's
     * always set instead of having to deal with possibly weird behaviour
     * for malicious applications setting only __O_SYNC.
     */
    if (flags & __O_SYNC)
        flags |= O_DSYNC;

    op->open_flag = flags;

    /* O_TRUNC implies we need access checks for write permissions */
    if (flags & O_TRUNC)
        acc_mode |= MAY_WRITE;

    /* Allow the LSM permission hook to distinguish append
       access from general write access. */
    if (flags & O_APPEND)
        acc_mode |= MAY_APPEND;

    op->acc_mode = acc_mode;

    op->intent = flags & O_PATH ? 0 : LOOKUP_OPEN;

    if (flags & O_CREAT) {
        op->intent |= LOOKUP_CREATE;
        if (flags & O_EXCL) {
            op->intent |= LOOKUP_EXCL;
            flags |= O_NOFOLLOW;
        }
    }

    if (flags & O_DIRECTORY)
        lookup_flags |= LOOKUP_DIRECTORY;
    if (!(flags & O_NOFOLLOW))
        lookup_flags |= LOOKUP_FOLLOW;

    if (how->resolve & RESOLVE_NO_XDEV)
        lookup_flags |= LOOKUP_NO_XDEV;
    if (how->resolve & RESOLVE_NO_MAGICLINKS)
        lookup_flags |= LOOKUP_NO_MAGICLINKS;
    if (how->resolve & RESOLVE_NO_SYMLINKS)
        lookup_flags |= LOOKUP_NO_SYMLINKS;
    if (how->resolve & RESOLVE_BENEATH)
        lookup_flags |= LOOKUP_BENEATH;
    if (how->resolve & RESOLVE_IN_ROOT)
        lookup_flags |= LOOKUP_IN_ROOT;

    op->lookup_flags = lookup_flags;
    return 0;
}

static long
do_sys_openat2(int dfd, const char *filename, struct open_how *how)
{
    struct open_flags op;
    struct filename *tmp;
    int fd = build_open_flags(how, &op);

    tmp = getname(filename);
    if (IS_ERR(tmp))
        return PTR_ERR(tmp);

    fd = get_unused_fd_flags(how->flags);
    if (fd >= 0) {
        struct file *f = do_filp_open(dfd, tmp, &op);
        if (IS_ERR(f)) {
            put_unused_fd(fd);
            fd = PTR_ERR(f);
        } else {
            printk("========== %s: fd(%d) file(%p)\n", __func__, fd, f);
            fd_install(fd, f);
        }
    }

    printk("%s: fd(%d) filename(%s) flags(%x) mode(%x)!\n",
          __func__, fd, filename, how->flags, how->mode);
    return fd;
}

static struct open_how build_open_how(int flags, umode_t mode)
{
    struct open_how how = {
        .flags = flags,
        .mode = mode,
    };

    return how;
}
EXPORT_SYMBOL(build_open_how);

/**
 * file_open_name - open file and return file pointer
 *
 * @name:   struct filename containing path to open
 * @flags:  open flags as per the open(2) second argument
 * @mode:   mode for the new file if O_CREAT is set, else ignored
 *
 * This is the helper to open a file from kernelspace if you really
 * have to.  But in generally you should not do this, so please move
 * along, nothing to see here..
 */
struct file *
file_open_name(struct filename *name, int flags, umode_t mode)
{
    struct open_flags op;
    struct open_how how = build_open_how(flags, mode);
    int err = build_open_flags(&how, &op);
    if (err)
        return ERR_PTR(err);
    return do_filp_open(AT_FDCWD, name, &op);
}

/**
 * filp_open - open file and return file pointer
 *
 * @filename:   path to open
 * @flags:  open flags as per the open(2) second argument
 * @mode:   mode for the new file if O_CREAT is set, else ignored
 *
 * This is the helper to open a file from kernelspace if you really
 * have to.  But in generally you should not do this, so please move
 * along, nothing to see here..
 */
struct file *filp_open(const char *filename, int flags, umode_t mode)
{
    struct filename *name = getname_kernel(filename);
    struct file *file = ERR_CAST(name);

    if (!IS_ERR(name)) {
        printk("####### %s: 1 filename(%s)\n", __func__, name->name);
        file = file_open_name(name, flags, mode);
        printk("####### %s: 2\n", __func__);
    }
    return file;
}
EXPORT_SYMBOL(filp_open);

/*
 * This is used by subsystems that don't want seekable
 * file descriptors. The function is not supposed to ever fail, the only
 * reason it returns an 'int' and not 'void' is so that it can be plugged
 * directly into file_operations structure.
 */
int nonseekable_open(struct inode *inode, struct file *filp)
{
    filp->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE);
    return 0;
}
EXPORT_SYMBOL(nonseekable_open);

long
_do_sys_open(int dfd, const char *filename, int flags, umode_t mode)
{
    struct open_how how = build_open_how(flags, mode);
    return do_sys_openat2(dfd, filename, &how);
}

long
_do_faccessat(int dfd, const char *filename, int mode, int flags)
{
    struct path path;
    struct inode *inode;
    int res;
    unsigned int lookup_flags = LOOKUP_FOLLOW;
    const struct cred *old_cred = NULL;

    if (mode & ~S_IRWXO)    /* where's F_OK, X_OK, W_OK, R_OK? */
        return -EINVAL;

    if (flags & ~(AT_EACCESS | AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH))
        return -EINVAL;

    if (flags & AT_SYMLINK_NOFOLLOW)
        lookup_flags &= ~LOOKUP_FOLLOW;
    if (flags & AT_EMPTY_PATH)
        lookup_flags |= LOOKUP_EMPTY;

#if 0
    if (!(flags & AT_EACCESS)) {
        old_cred = access_override_creds();
        if (!old_cred)
            return -ENOMEM;
    }
#endif

    res = user_path_at(dfd, filename, lookup_flags, &path);
    if (res)
        goto out;

    panic("%s: todo! filename(%s)",
          __func__, path.dentry->d_name.name);

 out:
#if 0
    if (old_cred)
        revert_creds(old_cred);
#endif

    return res;
}

void init_open(void)
{
    do_sys_open = _do_sys_open;
    do_faccessat = _do_faccessat;
}

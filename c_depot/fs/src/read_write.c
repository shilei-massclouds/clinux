// SPDX-License-Identifier: GPL-2.0

#include <fs.h>
#include <file.h>
#include <errno.h>
#include <namei.h>
#include <export.h>
#include <uaccess.h>
#include <syscalls.h>
#include <thread_info.h>

static ssize_t
new_sync_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
    ssize_t ret;
    struct kiocb kiocb;
    struct iov_iter iter;
    struct iovec iov = { .iov_base = buf, .iov_len = len };
    init_sync_kiocb(&kiocb, filp);
    kiocb.ki_pos = (ppos ? *ppos : 0);
    iov_iter_init(&iter, READ, &iov, 1, len);

    ret = call_read_iter(filp, &kiocb, &iter);
    BUG_ON(ret == -EIOCBQUEUED);
    if (ppos)
        *ppos = kiocb.ki_pos;
    return ret;
}

ssize_t
__kernel_read(struct file *file, void *buf, size_t count, loff_t *pos)
{
    ssize_t ret;
    mm_segment_t old_fs = get_fs();

    BUG_ON(!(file->f_mode & FMODE_READ));
    if (!(file->f_mode & FMODE_CAN_READ))
        panic("no FMODE_CAN_READ!");

    if (count > MAX_RW_COUNT)
        count = MAX_RW_COUNT;
    set_fs(KERNEL_DS);
    if (file->f_op->read)
        ret = file->f_op->read(file, (void *)buf, count, pos);
    else if (file->f_op->read_iter)
        ret = new_sync_read(file, (void *)buf, count, pos);
    else
        panic("bad read op!");

    set_fs(old_fs);
    return ret;
}

ssize_t
kernel_read(struct file *file, void *buf, size_t count, loff_t *pos)
{
    return __kernel_read(file, buf, count, pos);
}
EXPORT_SYMBOL(kernel_read);

long
_do_sys_readlinkat(int dfd, const char *pathname, char *buf, int bufsiz)
{
    int error;
    struct path path;
    int empty = 0;
    unsigned int lookup_flags = LOOKUP_EMPTY;

    if (bufsiz <= 0)
        return -EINVAL;

    error = user_path_at_empty(dfd, pathname, lookup_flags, &path, &empty);
    if (!error) {
        panic("%s: NOT implemented!", __func__);
    }
    return error;
}

/* file_ppos returns &file->f_pos or NULL if file is stream */
static inline loff_t *file_ppos(struct file *file)
{
    return file->f_mode & FMODE_STREAM ? NULL : &file->f_pos;
}

int
rw_verify_area(int read_write,
               struct file *file, const loff_t *ppos, size_t count)
{
    return 0;
}

ssize_t
vfs_write(struct file *file, const char *buf, size_t count, loff_t *pos)
{
    ssize_t ret;

    if (!(file->f_mode & FMODE_WRITE))
        return -EBADF;
    if (!(file->f_mode & FMODE_CAN_WRITE))
        return -EINVAL;
    if (unlikely(!access_ok(buf, count)))
        return -EFAULT;

    ret = rw_verify_area(WRITE, file, pos, count);
    if (ret)
        return ret;
    if (count > MAX_RW_COUNT)
        count = MAX_RW_COUNT;
    if (file->f_op->write)
        ret = file->f_op->write(file, buf, count, pos);
    else if (file->f_op->write_iter)
        panic("no write_iter!");
    else
        ret = -EINVAL;
    pr_debug("%s: ret(%ld)\n", __func__, ret);
    return ret;
}
EXPORT_SYMBOL(vfs_write);

long
_ksys_write(unsigned int fd, const char *buf, size_t count)
{
    ssize_t ret = -EBADF;
    struct fd f = fdget_pos(fd);

    if (f.file) {
        loff_t pos, *ppos = file_ppos(f.file);
        if (ppos) {
            pos = *ppos;
            ppos = &pos;
        }
        ret = vfs_write(f.file, buf, count, ppos);
        if (ret >= 0 && ppos)
            f.file->f_pos = pos;
    }
    return ret;
}

void init_read_write(void)
{
    do_sys_readlinkat = _do_sys_readlinkat;
    ksys_write = _ksys_write;
}

// SPDX-License-Identifier: GPL-2.0

#include <filemap.h>
#include <fs/ext2.h>

#define ext2_file_mmap  generic_file_mmap

const struct inode_operations ext2_file_inode_operations = {
    /*
    .listxattr  = ext2_listxattr,
    .getattr    = ext2_getattr,
    .setattr    = ext2_setattr,
    .get_acl    = ext2_get_acl,
    .set_acl    = ext2_set_acl,
    .fiemap     = ext2_fiemap,
    */
};

static ssize_t ext2_file_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
    return generic_file_read_iter(iocb, to);
}

const struct file_operations ext2_file_operations = {
    .open           = generic_file_open,
    .read_iter      = ext2_file_read_iter,
    .mmap           = ext2_file_mmap,
    /*
    .llseek         = generic_file_llseek,
    .write_iter     = ext2_file_write_iter,
    .unlocked_ioctl = ext2_ioctl,
    .release        = ext2_release_file,
    .fsync          = ext2_fsync,
    .get_unmapped_area = thp_get_unmapped_area,
    .splice_read    = generic_file_splice_read,
    .splice_write   = iter_file_splice_write,
    */
};


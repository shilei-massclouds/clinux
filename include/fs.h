/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FS_H
#define _LINUX_FS_H

#include <uio.h>
#include <cdev.h>
#include <cred.h>
#include <list.h>
#include <page.h>
#include <path.h>
#include <fcntl.h>
#include <types.h>
#include <xarray.h>
#include <mm_types.h>

/* high priority request, poll if possible */
#define RWF_HIPRI   ((__force __kernel_rwf_t)0x00000001)

typedef int __kernel_rwf_t;
typedef __kernel_rwf_t rwf_t;

#define INR_OPEN_CUR 1024   /* Initial setting for nfile rlimits */
#define INR_OPEN_MAX 4096   /* Hard limit for nfile rlimits */

#define BLOCK_SIZE_BITS 10
#define BLOCK_SIZE      (1<<BLOCK_SIZE_BITS)

#define SB_ACTIVE   (1<<30)

#define MAX_NON_LFS ((1UL<<31) - 1)

#define MAX_RW_COUNT (INT_MAX & PAGE_MASK)

/*
 * sb->s_flags.  Note that these mirror the equivalent MS_* flags where
 * represented in both.
 */
#define SB_RDONLY    1  /* Mount read-only */
#define SB_NOSUID    2  /* Ignore suid and sgid bits */
#define SB_NODEV     4  /* Disallow access to device special files */
#define SB_NOEXEC    8  /* Disallow program execution */
#define SB_SYNCHRONOUS  16  /* Writes are synced at once */
#define SB_MANDLOCK 64  /* Allow mandatory locks on an FS */
#define SB_DIRSYNC  128 /* Directory modifications are synchronous */
#define SB_NOATIME  1024    /* Do not update access times. */
#define SB_NODIRATIME   2048    /* Do not update directory access times */
#define SB_SILENT   32768
#define SB_POSIXACL (1<<16) /* VFS does not apply the umask */
#define SB_INLINECRYPT  (1<<17) /* Use blk-crypto for encrypted files */
#define SB_KERNMOUNT    (1<<22) /* this is a kern_mount call */
#define SB_I_VERSION    (1<<23) /* Update inode I_version field */
#define SB_LAZYTIME (1<<25) /* Update the on-disk [acm]times lazily */

#define SB_SUBMOUNT (1<<26)
#define SB_NOSEC    (1<<28)
#define SB_NOUSER   (1<<31)

#define WHITEOUT_DEV 0

#define ACC_MODE(x) ("\004\002\006\006"[(x)&O_ACCMODE])

struct super_block;
struct buffer_head;
struct readahead_control;
struct linux_binprm;
struct linux_binfmt;

/*
 * flags in file.f_mode.  Note that FMODE_READ and FMODE_WRITE must correspond
 * to O_WRONLY and O_RDWR via the strange trick in do_dentry_open()
 */

/* file is open for reading */
#define FMODE_READ      ((__force fmode_t)0x1)
/* file is open for writing */
#define FMODE_WRITE     ((__force fmode_t)0x2)
/* file is seekable */
#define FMODE_LSEEK     ((__force fmode_t)0x4)
/* file can be accessed using pread */
#define FMODE_PREAD     ((__force fmode_t)0x8)
/* file can be accessed using pwrite */
#define FMODE_PWRITE    ((__force fmode_t)0x10)
/* File is opened for execution with sys_execve / sys_uselib */
#define FMODE_EXEC      ((__force fmode_t)0x20)
/* File is opened with O_EXCL (only set for block devices) */
#define FMODE_EXCL      ((__force fmode_t)0x80)

/* File is huge (eg. /dev/kmem): treat loff_t as unsigned */
#define FMODE_UNSIGNED_OFFSET   ((__force fmode_t)0x2000)

/* File is opened with O_PATH; almost nothing can be done with it */
#define FMODE_PATH      ((__force fmode_t)0x4000)

/* File needs atomic accesses to f_pos */
#define FMODE_ATOMIC_POS        ((__force fmode_t)0x8000)

/* Has read method(s) */
#define FMODE_CAN_READ  ((__force fmode_t)0x20000)
#define FMODE_CAN_WRITE ((__force fmode_t)0x40000)
#define FMODE_OPENED    ((__force fmode_t)0x80000)
#define FMODE_CREATED   ((__force fmode_t)0x100000)
/* File is stream-like */
#define FMODE_STREAM    ((__force fmode_t)0x200000)
/* File was opened by fanotify and shouldn't generate fanotify events */
#define FMODE_NONOTIFY  ((__force fmode_t)0x4000000)

#define __FMODE_NONOTIFY    ((__force int) FMODE_NONOTIFY)

#define OPEN_FMODE(flag) \
    ((fmode_t)(((flag + 1) & O_ACCMODE) | (flag & __FMODE_NONOTIFY)))

#define __FMODE_EXEC    ((__force int) FMODE_EXEC)

#define MAY_EXEC        0x00000001
#define MAY_WRITE       0x00000002
#define MAY_READ        0x00000004
#define MAY_APPEND      0x00000008
#define MAY_ACCESS      0x00000010
#define MAY_OPEN        0x00000020
#define MAY_CHDIR       0x00000040

/*
 * Inode state bits.  Protected by inode->i_lock
 */
#define __I_NEW         3
#define I_NEW           (1 << __I_NEW)
#define I_CREATING      (1 << 15)

#define MAX_LFS_FILESIZE    ((loff_t)LLONG_MAX)

struct file;

struct address_space_operations {
    int (*readpage)(struct file *, struct page *);
    /*
     * Reads in the requested pages. Unlike ->readpage(), this is
     * PURELY used for read-ahead!.
     */
    int (*readpages)(struct file *filp, struct address_space *mapping,
                     struct list_head *pages, unsigned nr_pages);
    void (*readahead)(struct readahead_control *);
};

struct address_space {
    struct inode    *host;
    struct xarray   i_pages;
    unsigned long   nrpages;
    gfp_t           gfp_mask;
    atomic_t        i_mmap_writable;

    struct rb_root_cached   i_mmap;

    const struct address_space_operations *a_ops;
} __attribute__((aligned(sizeof(long))));

/*
 * Write life time hint values.
 * Stored in struct inode as u8.
 */
enum rw_hint {
    WRITE_LIFE_NOT_SET  = 0,
};

struct filename {
    const char *name;   /* pointer to actual string */
    const char iname[];
};

extern struct kmem_cache *names_cachep;

#define __getname() kmem_cache_alloc(names_cachep, GFP_KERNEL)

struct fs_context;

struct fs_context_operations {
    int (*get_tree)(struct fs_context *fc);
};

enum fs_context_purpose {
    FS_CONTEXT_FOR_MOUNT,       /* New superblock for explicit mount */
    FS_CONTEXT_FOR_SUBMOUNT,    /* New superblock for automatic submount */
    FS_CONTEXT_FOR_RECONFIGURE, /* Superblock reconfiguration (remount) */
};

struct fs_context {
    const struct fs_context_operations *ops;
    struct file_system_type *fs_type;
    void *fs_private;           /* The filesystem's context */
    struct dentry *root;        /* The root and superblock */
    void *s_fs_info;            /* Proposed s_fs_info */
    unsigned int sb_flags;      /* Proposed superblock flags (SB_*) */
    unsigned int sb_flags_mask; /* Superblock flags that were changed */
    const char *source;         /* The source name (eg. dev path) */
    enum fs_context_purpose purpose:8;
};

struct super_operations {
    struct inode *(*alloc_inode)(struct super_block *sb);
};

struct super_block {
    struct list_head s_list;    /* Keep this first */
    dev_t s_dev;                /* search index; _not_ kdev_t */
    struct dentry *s_root;
    unsigned long s_flags;
    void *s_fs_info;            /* Proposed s_fs_info */
    const struct super_operations *s_op;
    struct list_head s_inodes;  /* all inodes */
    struct hlist_node s_instances;
    struct block_device *s_bdev;
    struct backing_dev_info *s_bdi;
    struct file_system_type *s_type;
    char s_id[32];              /* Informational name */
    fmode_t s_mode;
    unsigned char s_blocksize_bits;
    unsigned long s_blocksize;
};

struct inode_operations {
    struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
    int (*mkdir) (struct inode *,struct dentry *,umode_t);
    int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
};

struct inode {
    umode_t             i_mode;

    /* Stat data, not accessed from path walking */
    unsigned long       i_ino;

    loff_t              i_size;
    dev_t               i_rdev;

    u8                  i_blkbits;
    blkcnt_t            i_blocks;

    struct hlist_node   i_hash;
    struct hlist_head   i_dentry;

    const struct inode_operations *i_op;
    const struct file_operations  *i_fop;

    struct super_block *i_sb;
    struct address_space *i_mapping;
    struct address_space i_data;

    unsigned long       i_state;
    struct list_head    i_sb_list;

    struct list_head    i_devices;
    union {
        struct block_device *i_bdev;
        struct cdev         *i_cdev;
    };

    atomic_t i_writecount;
};

struct pseudo_fs_context {
    const struct super_operations *ops;
    unsigned long magic;
};

struct kiocb {
    struct file *ki_filp;
    loff_t ki_pos;
};

struct file_operations {
    int (*open)(struct inode *, struct file *);
    ssize_t (*read)(struct file *, char *, size_t, loff_t *);
    ssize_t (*write)(struct file *, const char *, size_t, loff_t *);
    ssize_t (*read_iter)(struct kiocb *, struct iov_iter *);
    ssize_t (*write_iter)(struct kiocb *, struct iov_iter *);
    int (*mmap)(struct file *, struct vm_area_struct *);
};

struct file_system_type {
    const char *name;

    int fs_flags;
#define FS_REQUIRES_DEV     1
#define FS_BINARY_MOUNTDATA 2
#define FS_HAS_SUBTYPE      4
#define FS_USERNS_MOUNT     8   /* Can be mounted by userns root */
#define FS_DISALLOW_NOTIFY_PERM 16  /* Disable fanotify permission events */
#define FS_RENAME_DOES_D_MOVE   32768 /* FS will handle d_move() during rename() internally. */

    int (*init_fs_context)(struct fs_context *);

    struct dentry *(*mount)(struct file_system_type *,
                            int, const char *);

    struct file_system_type *next;
    struct hlist_head fs_supers;
};

struct fs_struct {
    struct path root;
    struct path pwd;
};

/*
 * open.c
 */
struct open_flags {
    int open_flag;
    umode_t mode;
    int acc_mode;
    int intent;
    int lookup_flags;
};

/*
 * Track a single file's readahead state
 */
struct file_ra_state {
    pgoff_t start;          /* where readahead started */
    unsigned int size;      /* # of readahead pages */
    unsigned int async_size;    /* do asynchronous readahead
                                   when there are only # of pages ahead */

    unsigned int ra_pages;  /* Maximum readahead window */
    loff_t prev_pos;        /* Cache last read() position */
};

struct file {
    struct path     f_path;
    struct inode    *f_inode;   /* cached value */
    unsigned int    f_flags;
    fmode_t         f_mode;
    loff_t          f_pos;

    struct file_ra_state f_ra;

    struct address_space *f_mapping;
    const struct file_operations *f_op;

    /* needed for tty driver, and maybe others */
    void *private_data;
};

extern bool rootfs_initialized;

struct fs_context *
fs_context_for_mount(struct file_system_type *fs_type,
                     unsigned int sb_flags);

void
put_fs_context(struct fs_context *fc);

void
set_fs_root(struct fs_struct *fs, const struct path *path);

void
set_fs_pwd(struct fs_struct *fs, const struct path *path);

static inline void
get_fs_root(struct fs_struct *fs, struct path *root)
{
    *root = fs->root;
}

static inline struct file_system_type *
get_filesystem(struct file_system_type *fs)
{
    return fs;
}

int
get_tree_nodev(struct fs_context *fc,
               int (*fill_super)(struct super_block *sb,
                                 struct fs_context *fc));

struct inode *
new_inode(struct super_block *sb);

static inline void
iput(struct inode *inode)
{
}

struct dentry *
simple_lookup(struct inode *dir,
              struct dentry *dentry,
              unsigned int flags);

int
init_mkdir(const char *pathname, umode_t mode);

int
get_filesystem_list(char *buf);

int
register_filesystem(struct file_system_type * fs);

struct file_system_type *
get_fs_type(const char *name);

struct dentry *
mount_bdev(struct file_system_type *fs_type,
           int flags, const char *dev_name,
           int (*fill_super)(struct super_block *, void *, int));

struct block_device *
blkdev_get_by_path(const char *path, fmode_t mode, void *holder);

struct inode *
iget_locked(struct super_block *sb, unsigned long ino);

struct inode *
iget5_locked(struct super_block *sb, unsigned long hashval,
             int (*test)(struct inode *, void *),
             int (*set)(struct inode *, void *), void *data);

void
bdev_cache_init(void);

struct pseudo_fs_context *
init_pseudo(struct fs_context *fc, unsigned long magic);

void inode_init_once(struct inode *inode);

void inode_init(void);

static inline int inode_unhashed(struct inode *inode)
{
    return hlist_unhashed(&inode->i_hash);
}

static inline void
i_size_write(struct inode *inode, loff_t i_size)
{
    inode->i_size = i_size;
}

struct super_block *
sget(struct file_system_type *type,
     int (*test)(struct super_block *,void *),
     int (*set)(struct super_block *,void *),
     int flags,
     void *data);

int sb_set_blocksize(struct super_block *sb, int size);

int sb_min_blocksize(struct super_block *sb, int size);

static inline loff_t i_size_read(const struct inode *inode)
{
    return inode->i_size;
}

int init_chdir(const char *filename);

int init_chroot(const char *filename);

int kernel_execve(const char *kernel_filename,
                  const char *const *argv, const char *const *envp);

struct file *do_filp_open(int dfd, struct filename *pathname,
                          const struct open_flags *op);

struct file *alloc_empty_file(int flags, const struct cred *cred);

struct mount *__lookup_mnt(struct vfsmount *mnt, struct dentry *dentry);

typedef int (get_block_t)(struct inode *inode, sector_t iblock,
                          struct buffer_head *bh_result, int create);

int mpage_readpage(struct page *page, get_block_t get_block);

void mpage_readahead(struct readahead_control *rac, get_block_t get_block);

int bdev_read_page(struct block_device *bdev, sector_t sector,
                   struct page *page);

int vfs_open(const struct path *path, struct file *file);

int generic_file_open(struct inode *inode, struct file *filp);

ssize_t
kernel_read(struct file *file, void *buf, size_t count, loff_t *pos);

static inline void
init_sync_kiocb(struct kiocb *kiocb, struct file *filp)
{
    *kiocb = (struct kiocb) {
        .ki_filp = filp,
    };
}

static inline ssize_t
call_read_iter(struct file *file, struct kiocb *kio, struct iov_iter *iter)
{
    return file->f_op->read_iter(kio, iter);
}

void
page_cache_sync_readahead(struct address_space *mapping,
                          struct file_ra_state *ra, struct file *filp,
                          pgoff_t index, unsigned long req_count);

void
file_ra_state_init(struct file_ra_state *ra, struct address_space *mapping);

extern struct super_block *blockdev_superblock;
static inline bool sb_is_blkdev_sb(struct super_block *sb)
{
    return sb == blockdev_superblock;
}

typedef struct block_device *
(*I_BDEV_T)(struct inode *inode);
extern I_BDEV_T I_BDEV;

int begin_new_exec(struct linux_binprm *bprm);

void setup_new_exec(struct linux_binprm * bprm);

int setup_arg_pages(struct linux_binprm *bprm,
                    unsigned long stack_top,
                    int executable_stack);

static inline struct inode *file_inode(const struct file *f)
{
    return f->f_inode;
}

static inline int call_mmap(struct file *file, struct vm_area_struct *vma)
{
    return file->f_op->mmap(file, vma);
}

void set_binfmt(struct linux_binfmt *new);

void finalize_exec(struct linux_binprm *bprm);

struct file *filp_open(const char *filename, int flags, umode_t mode);

int init_dup(struct file *file);

/* Alas, no aliases. Too much hassle with bringing module.h everywhere */
#define fops_get(fops) fops

/*
 * This one is to be used *ONLY* from ->open() instances.
 * fops must be non-NULL, pinned down *and* module dependencies
 * should be sufficient to pin the caller down as well.
 */
#define replace_fops(f, fops) \
    do {    \
        struct file *__file = (f); \
        BUG_ON(!(__file->f_op = (fops))); \
    } while(0)

ssize_t
vfs_write(struct file *file, const char *buf, size_t count, loff_t *pos);

int nonseekable_open(struct inode *inode, struct file *filp);

extern struct file * open_exec(const char *);

enum positive_aop_returns {
    AOP_WRITEPAGE_ACTIVATE  = 0x80000,
    AOP_TRUNCATED_PAGE  = 0x80001,
};

ssize_t
rw_copy_check_uvector(int type, const struct iovec *uvector,
                      unsigned long nr_segs, unsigned long fast_segs,
                      struct iovec *fast_pointer,
                      struct iovec **ret_pointer);

#endif /* _LINUX_FS_H */

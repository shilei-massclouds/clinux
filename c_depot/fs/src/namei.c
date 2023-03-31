// SPDX-License-Identifier: GPL-2.0

#include <slab.h>
#include <stat.h>
#include <errno.h>
#include <mount.h>
#include <namei.h>
#include <dcache.h>
#include <export.h>
#include <limits.h>
#include <string.h>
#include <current.h>
#include <stringhash.h>

#define EMBEDDED_NAME_MAX (PATH_MAX - offsetof(struct filename, iname))

struct nameidata {
    struct path path;
    struct qstr last;
    struct path root;
    struct inode *inode; /* path.dentry.d_inode */
    unsigned int flags;
    int last_type;
    struct filename *name;
    int dfd;
};

/*
 * We know there's a real path component here of at least
 * one character.
 */
static inline u64
hash_name(const void *salt, const char *name)
{
    unsigned long hash = init_name_hash(salt);
    unsigned long len = 0, c;

    c = (unsigned char)*name;
    do {
        len++;
        hash = partial_name_hash(c, hash);
        c = (unsigned char)name[len];
    } while (c && c != '/');
    return hashlen_create(end_name_hash(hash), len);
}

struct filename *
getname_kernel(const char *filename)
{
    struct filename *result;
    int len = strlen(filename) + 1;

    result = __getname();
    if (unlikely(!result))
        return ERR_PTR(-ENOMEM);

    if (len <= EMBEDDED_NAME_MAX) {
        result->name = (char *)result->iname;
    } else if (len <= PATH_MAX) {
        const size_t size = offsetof(struct filename, iname[1]);
        struct filename *tmp;

        tmp = kmalloc(size, GFP_KERNEL);
        if (unlikely(!tmp)) {
            return ERR_PTR(-ENOMEM);
        }
        tmp->name = (char *)result;
        result = tmp;
    } else {
        return ERR_PTR(-ENAMETOOLONG);
    }

    memcpy((char *)result->name, filename, len);
    return result;
}

static int
set_root(struct nameidata *nd)
{
    struct fs_struct *fs = current->fs;

    if (nd->flags & LOOKUP_RCU) {
        printk("%s: 1\n", __func__);
        nd->root = fs->root;
    } else {
        get_fs_root(fs, &nd->root);
        nd->flags |= LOOKUP_ROOT_GRABBED;
    }
    return 0;
}

static int
nd_jump_root(struct nameidata *nd)
{
    if (!nd->root.mnt) {
        int error = set_root(nd);
        if (error)
            return error;
    }
    nd->path = nd->root;
    nd->inode = nd->path.dentry->d_inode;
    nd->flags |= LOOKUP_JUMPED;
    printk("%s: dir(%s)\n",
           __func__, nd->root.dentry->d_name.name);
    return 0;
}

static const char *
path_init(struct nameidata *nd, unsigned flags)
{
    const char *s = nd->name->name;

    if (!*s)
        flags &= ~LOOKUP_RCU;

    if (flags & LOOKUP_ROOT)
        panic("no LOOKUP_ROOT!");

    nd->flags = flags | LOOKUP_JUMPED;

    nd->root.mnt = NULL;
    nd->path.mnt = NULL;
    nd->path.dentry = NULL;

    printk(">>>>>> %s: 0 name(%s) flags(%x)\n", __func__, s, flags);

    if (*s == '/' && !(flags & LOOKUP_IN_ROOT)) {
        int error = nd_jump_root(nd);
        if (unlikely(error))
            return ERR_PTR(error);
        return s;
    }

    /* Relative pathname -- get the starting-point it is relative to. */
    if (nd->dfd == AT_FDCWD) {
        if (flags & LOOKUP_RCU) {
            struct fs_struct *fs = current->fs;
            nd->path = fs->pwd;
            printk("%s: >>>>> dir(%s)\n",
                   __func__, fs->pwd.dentry->d_name.name);

            nd->inode = nd->path.dentry->d_inode;
        } else {
            panic("no LOOKUP_RCU!");
        }
    } else {
        panic("no AT_FDCMD!");
    }

    return s;
}

static int unlazy_walk(struct nameidata *nd)
{
    struct dentry *parent = nd->path.dentry;
    BUG_ON(!(nd->flags & LOOKUP_RCU));

    nd->flags &= ~LOOKUP_RCU;
    BUG_ON(nd->inode != parent->d_inode);
    return 0;
}

static struct dentry *
lookup_fast(struct nameidata *nd, struct inode **inode)
{
    struct dentry *dentry;
    struct dentry *parent = nd->path.dentry;

    if (nd->flags & LOOKUP_RCU) {
        dentry = __d_lookup_rcu(parent, &nd->last);
        if (unlikely(!dentry)) {
            if (unlazy_walk(nd))
                return ERR_PTR(-ECHILD);
            return NULL;
        }
    } else {
        dentry = __d_lookup(parent, &nd->last);
        if (unlikely(!dentry))
            return NULL;
    }

    *inode = d_backing_inode(dentry);
    return dentry;
}

static bool
__follow_mount_rcu(struct nameidata *nd, struct path *path,
                   struct inode **inode)
{
    struct dentry *dentry = path->dentry;
    unsigned int flags = dentry->d_flags;

    if (likely(!(flags & DCACHE_MANAGED_DENTRY)))
        return true;

    if (unlikely(nd->flags & LOOKUP_NO_XDEV))
        return false;

    for (;;) {
        if (flags & DCACHE_MOUNTED) {
            struct mount *mounted = __lookup_mnt(path->mnt, dentry);
            if (mounted) {
                path->mnt = &mounted->mnt;
                dentry = path->dentry = mounted->mnt.mnt_root;
                nd->flags |= LOOKUP_JUMPED;
                *inode = dentry->d_inode;
                /*
                 * We don't need to re-check ->d_seq after this
                 * ->d_inode read - there will be an RCU delay
                 * between mount hash removal and ->mnt_root
                 * becoming unpinned.
                 */
                flags = dentry->d_flags;
                continue;
            }
        } else {
            panic("no DCACHE_MOUNTED!");
        }
        return !(flags & DCACHE_NEED_AUTOMOUNT);
    }
    panic("%s: !", __func__);
}

static inline int
traverse_mounts(struct path *path, bool *jumped,
                unsigned lookup_flags)
{
    unsigned flags = path->dentry->d_flags;

    /* fastpath */
    if (likely(!(flags & DCACHE_MANAGED_DENTRY))) {
        *jumped = false;
        printk("--- %s: 1 flags(%x)\n", __func__, flags);
        if (unlikely(d_flags_negative(flags)))
            return -ENOENT;
        return 0;
    }
    panic("%s: todo!", __func__);
    //return __traverse_mounts(path, flags, jumped, lookup_flags);
}

static inline int
handle_mounts(struct nameidata *nd, struct dentry *dentry,
              struct path *path, struct inode **inode)
{
    bool jumped;
    int ret;

    path->mnt = nd->path.mnt;
    path->dentry = dentry;
    printk("%s: 1 dir(%s) flags(%u)\n",
           __func__, path->dentry->d_name.name, nd->flags);
    if (nd->flags & LOOKUP_RCU) {
        if (unlikely(!*inode))
            return -ENOENT;
        if (likely(__follow_mount_rcu(nd, path, inode)))
            return 0;
        path->mnt = nd->path.mnt;
        path->dentry = dentry;
    }

    ret = traverse_mounts(path, &jumped, nd->flags);
    if (jumped) {
        if (unlikely(nd->flags & LOOKUP_NO_XDEV))
            ret = -EXDEV;
        else
            nd->flags |= LOOKUP_JUMPED;
    }
    if (unlikely(ret)) {
#if 0
        dput(path->dentry);
        if (path->mnt != nd->path.mnt)
            mntput(path->mnt);
#endif
    } else {
        *inode = d_backing_inode(path->dentry);
    }
    return ret;
}

static const char *
step_into(struct nameidata *nd, struct dentry *dentry, struct inode *inode)
{
    struct path path;
    int err = handle_mounts(nd, dentry, &path, &inode);
    if (err < 0)
        return ERR_PTR(err);

    printk("%s: dentry(%s) inode(%p)\n",
           __func__, dentry->d_name.name, inode);

    nd->path = path;
    nd->inode = inode;
    return NULL;
}

static const char *handle_dots(struct nameidata *nd, int type)
{
    if (type == LAST_DOTDOT) {
        panic("no support for dotdot!");
    }
    return NULL;
}

/* Fast lookup failed, do it the slow way */
static struct dentry *
__lookup_slow(const struct qstr *name, struct dentry *dir,
              unsigned int flags)
{
    struct dentry *dentry, *old;
    struct inode *inode = dir->d_inode;

    printk("%s: 0 qstr(%s) dir(%s)\n",
           __func__, name->name, dir->d_name.name);

    dentry = d_alloc_parallel(dir, name);
    if (IS_ERR(dentry))
        return dentry;

    if (unlikely(!d_in_lookup(dentry))) {
        panic("not in lookup!");
    } else {
        printk("%s: 1 dir(%s) inode(%p)\n",
               __func__, dir->d_name.name, inode);
        old = inode->i_op->lookup(inode, dentry, flags);
        printk("%s: 2\n", __func__);
        d_lookup_done(dentry);
        if (unlikely(old))
            dentry = old;
    }
    return dentry;
}

static struct dentry *
lookup_slow(const struct qstr *name, struct dentry *dir,
            unsigned int flags)
{
    return __lookup_slow(name, dir, flags);
}

static const char *
walk_component(struct nameidata *nd)
{
    struct inode *inode;
    struct dentry *dentry;

    printk("%s: filename(%s) last(%s) dir(%s)\n",
           __func__, nd->name->name, nd->last.name,
           nd->path.dentry->d_name.name);

    /*
     * "." and ".." are special - ".." especially so because it has
     * to be able to know about the current root directory and
     * parent relationships.
     */
    if (unlikely(nd->last_type != LAST_NORM))
        return handle_dots(nd, nd->last_type);

    dentry = lookup_fast(nd, &inode);
    if (IS_ERR(dentry))
        return ERR_CAST(dentry);
    if (unlikely(!dentry)) {
        dentry = lookup_slow(&nd->last, nd->path.dentry, nd->flags);
        printk("%s: 1 dentry(%p)\n", __func__, dentry);
        if (IS_ERR(dentry))
            return ERR_CAST(dentry);
    }

    printk("%s 2: filename(%s) last(%s) dir(%s)\n",
           __func__, nd->name->name, nd->last.name,
           dentry->d_name.name);
    return step_into(nd, dentry, inode);
}

static int
link_path_walk(const char *name, struct nameidata *nd)
{
    int err;
    int depth = 0; // depth <= nd->depth

    nd->last_type = LAST_ROOT;
    nd->flags |= LOOKUP_PARENT;
    if (IS_ERR(name))
        return PTR_ERR(name);
    while (*name=='/')
        name++;
    if (!*name)
        return 0;

    for (;;) {
        int type;
        u64 hash_len;
        const char *link;

        hash_len = hash_name(nd->path.dentry, name);
        type = LAST_NORM;

        if (name[0] == '.') switch (hashlen_len(hash_len)) {
            case 2:
                if (name[1] == '.') {
                    type = LAST_DOTDOT;
                    nd->flags |= LOOKUP_JUMPED;
                }
                break;
            case 1:
                type = LAST_DOT;
        }

        nd->last.hash_len = hash_len;
        nd->last.name = name;
        nd->last_type = type;

        name += hashlen_len(hash_len);
        if (!*name) {
            nd->flags &= ~LOOKUP_PARENT;
            return 0;
        }

        do {
            name++;
        } while (unlikely(*name == '/'));

        if (unlikely(!*name))
            panic("bad name!");

        link = walk_component(nd);
        if (unlikely(link)) {
            if (IS_ERR(link))
                return PTR_ERR(link);
            panic("bad link!");
        }
        if (unlikely(!d_can_lookup(nd->path.dentry))) {
            if (nd->flags & LOOKUP_RCU) {
                if (unlazy_walk(nd))
                    return -ECHILD;
            }
            return -ENOTDIR;
        }
    }

    return 0;
}

/* Returns 0 and nd will be valid on success; Retuns error, otherwise. */
static int
path_parentat(struct nameidata *nd, unsigned flags, struct path *parent)
{
    const char *s = path_init(nd, flags);
    int err = link_path_walk(s, nd);
    if (!err) {
        *parent = nd->path;
        nd->path.mnt = NULL;
        nd->path.dentry = NULL;
    }
    return err;
}

static void
set_nameidata(struct nameidata *p, int dfd, struct filename *name)
{
    p->dfd = dfd;
    p->name = name;
}

static struct filename *
filename_parentat(int dfd, struct filename *name,
                  unsigned int flags, struct path *parent,
                  struct qstr *last, int *type)
{
    int retval;
    struct nameidata nd;

    if (IS_ERR(name))
        return name;

    set_nameidata(&nd, dfd, name);
    retval = path_parentat(&nd, flags | LOOKUP_RCU, parent);
    if (likely(!retval)) {
        *last = nd.last;
        *type = nd.last_type;
    } else {
        name = ERR_PTR(retval);
    }

    return name;
}

static struct dentry *
lookup_dcache(const struct qstr *name,
              struct dentry *dir,
              unsigned int flags)
{
    struct dentry *dentry = d_lookup(dir, name);
    return dentry;
}

static struct dentry *
__lookup_hash(const struct qstr *name,
              struct dentry *base,
              unsigned int flags)
{
    struct dentry *old;
    struct inode *dir = base->d_inode;
    struct dentry *dentry = lookup_dcache(name, base, flags);

    if (dentry)
        return dentry;

    dentry = d_alloc(base, name);
    if (unlikely(!dentry))
        return ERR_PTR(-ENOMEM);

    old = dir->i_op->lookup(dir, dentry, flags);
    BUG_ON(old);

    return dentry;
}

static struct dentry *
filename_create(int dfd, struct filename *name,
                struct path *path, unsigned int lookup_flags)
{
    int type;
    struct qstr last;
    struct dentry *dentry;
    bool is_dir = (lookup_flags & LOOKUP_DIRECTORY);

    /*
     * Note that only LOOKUP_REVAL and LOOKUP_DIRECTORY matter here. Any
     * other flags passed in are ignored!
     */
    lookup_flags &= LOOKUP_REVAL;

    name = filename_parentat(dfd, name, lookup_flags, path, &last, &type);
    if (IS_ERR(name))
        return ERR_CAST(name);

    dentry = __lookup_hash(&last, path->dentry, lookup_flags);
    if (IS_ERR(dentry))
        panic("cannot lookup hash!");

    return dentry;
}

struct dentry *
kern_path_create(int dfd, const char *pathname,
                 struct path *path, unsigned int lookup_flags)
{
    return filename_create(dfd, getname_kernel(pathname),
                           path, lookup_flags);
}
EXPORT_SYMBOL(kern_path_create);

int
vfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    if (!dir->i_op->mkdir)
        return -EPERM;

    mode &= (S_IRWXUGO|S_ISVTX);
    return dir->i_op->mkdir(dir, dentry, mode);
}
EXPORT_SYMBOL(vfs_mkdir);

int
vfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
    if (!dir->i_op->mknod)
        return -EPERM;

    return dir->i_op->mknod(dir, dentry, mode, dev);
}
EXPORT_SYMBOL(vfs_mknod);

static inline const char *
lookup_last(struct nameidata *nd)
{
    if (nd->last_type == LAST_NORM && nd->last.name[nd->last.len])
        nd->flags |= LOOKUP_FOLLOW | LOOKUP_DIRECTORY;

    return walk_component(nd);
}

static int
path_lookupat(struct nameidata *nd, unsigned flags, struct path *path)
{
    int err;
    const char *s = path_init(nd, flags);

    while (!(err = link_path_walk(s, nd)) &&
           (s = lookup_last(nd)) != NULL)
        ;

    if (!err) {
        *path = nd->path;
        nd->path.mnt = NULL;
        nd->path.dentry = NULL;
    }
    return err;
}

int
filename_lookup(int dfd, struct filename *name, unsigned flags,
                struct path *path, struct path *root)
{
    int retval;
    struct nameidata nd;
    if (IS_ERR(name))
        return PTR_ERR(name);
    if (unlikely(root)) {
        nd.root = *root;
        flags |= LOOKUP_ROOT;
    }
    set_nameidata(&nd, dfd, name);
    retval = path_lookupat(&nd, flags | LOOKUP_RCU, path);
    printk("%s: %s %d\n", __func__, name->name, retval);
    return retval;
}

int
kern_path(const char *name, unsigned int flags, struct path *path)
{
    return filename_lookup(AT_FDCWD, getname_kernel(name),
                           flags, path, NULL);
}
EXPORT_SYMBOL(kern_path);

static struct dentry *
lookup_open(struct nameidata *nd, struct file *file,
            const struct open_flags *op, bool got_write)
{
    struct dentry *dentry;
    struct dentry *dir = nd->path.dentry;
    struct inode *dir_inode = dir->d_inode;

    printk("### %s: (%p)\n", __func__, nd->path.dentry->d_inode);

    file->f_mode &= ~FMODE_CREATED;
    dentry = d_lookup(dir, &nd->last);
    for (;;) {
        if (!dentry) {
            dentry = d_alloc_parallel(dir, &nd->last);
            if (IS_ERR(dentry))
                panic("bad dentry!");
        }

        if (d_in_lookup(dentry))
            break;

        panic("%s: 1", __func__);
    }

    if (dentry->d_inode) {
        /* Cached positive dentry: will open in f_op->open */
        return dentry;
    }

    if (d_in_lookup(dentry)) {
        struct dentry *res =
            dir_inode->i_op->lookup(dir_inode, dentry, nd->flags);
        d_lookup_done(dentry);
        if (unlikely(res)) {
            if (IS_ERR(res))
                panic("lookup error!");
            dentry = res;
        }
    }

    printk("%s: dentry(%s)\n", __func__, dentry->d_name.name);
    return dentry;
}

static const char *
open_last_lookups(struct nameidata *nd,
                  struct file *file, const struct open_flags *op)
{
    const char *res;
    struct inode *inode;
    struct dentry *dentry;
    int open_flag = op->open_flag;

    nd->flags |= op->intent;

    if (nd->last_type != LAST_NORM) {
        panic("NOT NORM!");
        return handle_dots(nd, nd->last_type);
    }

    if (!(open_flag & O_CREAT)) {
        if (nd->last.name[nd->last.len])
            nd->flags |= LOOKUP_FOLLOW | LOOKUP_DIRECTORY;
        /* we _can_ be in RCU mode here */
        dentry = lookup_fast(nd, &inode);
        if (IS_ERR(dentry))
            panic("bad dentry!");
        if (likely(dentry))
            goto finish_lookup;
    } else {
        panic("create things! open_flag(%d) (%s)",
              open_flag, nd->last.name);
    }

    dentry = lookup_open(nd, file, op, false);

finish_lookup:
    res = step_into(nd, dentry, inode);
    if (unlikely(res))
        nd->flags &= ~(LOOKUP_OPEN|LOOKUP_CREATE|LOOKUP_EXCL);
    return res;
}

/*
 * Handle the last step of open()
 */
static int do_open(struct nameidata *nd,
                   struct file *file, const struct open_flags *op)
{
    return vfs_open(&nd->path, file);
}

static struct file *
path_openat(struct nameidata *nd,
            const struct open_flags *op, unsigned flags)
{
    int error;
    struct file *file;

    file = alloc_empty_file(op->open_flag, NULL);
    if (IS_ERR(file))
        return file;

    if (unlikely(file->f_flags & __O_TMPFILE)) {
        panic("__O_TMPFILE");
    } else if (unlikely(file->f_flags & O_PATH)) {
        panic("O_PATH");
    } else {
        const char *s = path_init(nd, flags);
        printk("####### %s: 1 (%s)\n", __func__, s);
        while (!(error = link_path_walk(s, nd)) &&
               (s = open_last_lookups(nd, file, op)) != NULL)
            ;
        printk("####### %s: 2 (%s)\n", __func__, s);
        if (!error)
            error = do_open(nd, file, op);
        printk("####### %s: 3 ret(%d)\n", __func__, error);
    }

    if (likely(!error)) {
        if (likely(file->f_mode & FMODE_OPENED))
            return file;
        BUG_ON(1);
        panic("open error!");
    }

    panic("%s: !", __func__);
}

struct file *do_filp_open(int dfd, struct filename *pathname,
                          const struct open_flags *op)
{
    struct file *filp;
    struct nameidata nd;
    int flags = op->lookup_flags;

    set_nameidata(&nd, dfd, pathname);
    printk("####### %s: 1 filename(%s)\n", __func__, pathname->name);
    filp = path_openat(&nd, op, flags | LOOKUP_RCU);
    printk("####### %s: 2 filename(%s)\n", __func__, pathname->name);
    return filp;
}
EXPORT_SYMBOL(do_filp_open);

struct filename *
getname_flags(const char *filename, int flags, int *empty)
{
    int len;
    char *kname;
    struct filename *result;

    result = __getname();
    if (unlikely(!result))
        return ERR_PTR(-ENOMEM);

    /*
     * First, try to embed the struct filename inside the names_cache
     * allocation
     */
    kname = (char *)result->iname;
    result->name = kname;

    len = strncpy_from_user(kname, filename, EMBEDDED_NAME_MAX);
    if (unlikely(len < 0))
        panic("strncpy from user error!");

    /*
     * Uh-oh. We have a name that's approaching PATH_MAX. Allocate a
     * separate struct filename so we can dedicate the entire
     * names_cache allocation for the pathname, and re-do the copy from
     * userland.
     */
    if (unlikely(len == EMBEDDED_NAME_MAX)) {
        const size_t size = offsetof(struct filename, iname[1]);
        kname = (char *)result;

        /*
         * size is chosen that way we to guarantee that
         * result->iname[0] is within the same object and that
         * kname can't be equal to result->iname, no matter what.
         */
        result = kzalloc(size, GFP_KERNEL);
        if (unlikely(!result))
            panic("out of memory!");

        result->name = kname;
        len = strncpy_from_user(kname, filename, PATH_MAX);
        if (unlikely(len < 0))
            panic("out of memory!");

        if (unlikely(len == PATH_MAX))
            panic("out of memory!");
    }

    /* The empty path is special. */
    if (unlikely(!len)) {
        if (empty)
            *empty = 1;
        if (!(flags & LOOKUP_EMPTY))
            return ERR_PTR(-ENOENT);
    }
    return result;
}

int
user_path_at_empty(int dfd, const char *name, unsigned flags,
                   struct path *path, int *empty)
{
    return filename_lookup(dfd, getname_flags(name, flags, empty),
                           flags, path, NULL);
}
EXPORT_SYMBOL(user_path_at_empty);

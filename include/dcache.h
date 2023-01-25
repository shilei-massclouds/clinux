/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_DCACHE_H
#define _LINUX_DCACHE_H

#include <fs.h>
#include <list_bl.h>

#define IS_ROOT(x) ((x) == (x)->d_parent)

#define DNAME_INLINE_LEN 32 /* 192 bytes */

#define DCACHE_OP_COMPARE       0x00000002
#define DCACHE_MOUNTED          0x00010000 /* is a mountpoint */
#define DCACHE_NEED_AUTOMOUNT   0x00020000 /* handle automount on this dir */
#define DCACHE_MISS_TYPE        0x00000000 /* Negative dentry (maybe fallthru to nowhere) */
#define DCACHE_DIRECTORY_TYPE   0x00200000 /* Normal directory */
#define DCACHE_REGULAR_TYPE     0x00400000 /* Regular file type (or fallthru to such) */
#define DCACHE_SPECIAL_TYPE     0x00500000 /* Other file type (or fallthru to such) */
#define DCACHE_ENTRY_TYPE       0x00700000
#define DCACHE_FALLTHRU         0x01000000 /* Fall through to lower layer */
#define DCACHE_PAR_LOOKUP       0x10000000 /* being looked up (with parent locked shared) */
#define DCACHE_MANAGE_TRANSIT   0x00040000 /* manage transit from this dirent */
#define DCACHE_MANAGED_DENTRY \
    (DCACHE_MOUNTED|DCACHE_NEED_AUTOMOUNT|DCACHE_MANAGE_TRANSIT)

#define HASH_LEN_DECLARE u32 hash; u32 len
#define bytemask_from_count(cnt)   (~(~0ul << (cnt)*8))

struct qstr {
    union {
        struct {
            HASH_LEN_DECLARE;
        };
        u64 hash_len;
    };
    const unsigned char *name;
};

#define QSTR_INIT(n,l) { { { .len = l } }, .name = n }

struct dentry {
    unsigned int d_flags;           /* protected by d_lock */
    struct hlist_bl_node d_hash;    /* lookup hash list */
    struct dentry *d_parent;        /* parent directory */
    struct qstr d_name;
    struct inode *d_inode;  /* Where the name belongs to - NULL is negative */

    unsigned char d_iname[DNAME_INLINE_LEN];    /* small names */
    struct super_block *d_sb;   /* The root of the dentry tree */
    struct list_head d_child;   /* child of parent list */
    struct list_head d_subdirs; /* our children */
    struct hlist_bl_node d_in_lookup_hash;  /* only for in-lookup ones */
    struct hlist_node d_alias;  /* inode alias list */
};

static inline struct dentry *
dget(struct dentry *dentry)
{
    return dentry;
}

struct dentry *
d_make_root(struct inode *root_inode);

struct dentry *
d_lookup(const struct dentry *parent, const struct qstr *name);

struct dentry *
__d_lookup(const struct dentry *parent, const struct qstr *name);

struct dentry *
__d_lookup_rcu(const struct dentry *parent, const struct qstr *name);

struct dentry *
d_alloc(struct dentry * parent, const struct qstr *name);

void
d_instantiate(struct dentry *entry, struct inode * inode);

static inline struct inode *
d_backing_inode(const struct dentry *upper)
{
    return upper->d_inode;
}

void
d_add(struct dentry *entry, struct inode *inode);

static inline bool d_mountpoint(const struct dentry *dentry)
{
    return dentry->d_flags & DCACHE_MOUNTED;
}

/**
 *  d_unhashed -    is dentry hashed
 *  @dentry: entry to check
 *
 *  Returns true if the dentry passed is not currently hashed.
 */
static inline int
d_unhashed(const struct dentry *dentry)
{
    return hlist_bl_unhashed(&dentry->d_hash);
}

static inline int d_unlinked(const struct dentry *dentry)
{
    return d_unhashed(dentry) && !IS_ROOT(dentry);
}

struct dentry *
d_alloc_parallel(struct dentry *parent, const struct qstr *name);

static inline int d_in_lookup(const struct dentry *dentry)
{
    return dentry->d_flags & DCACHE_PAR_LOOKUP;
}

extern void __d_lookup_done(struct dentry *);

static inline void d_lookup_done(struct dentry *dentry)
{
    if (unlikely(d_in_lookup(dentry)))
        __d_lookup_done(dentry);
}

int d_set_mounted(struct dentry *dentry);

struct dentry *d_splice_alias(struct inode *inode, struct dentry *dentry);

/*
 * Directory cache entry type accessor functions.
 */
static inline unsigned __d_entry_type(const struct dentry *dentry)
{
    return dentry->d_flags & DCACHE_ENTRY_TYPE;
}

static inline bool d_can_lookup(const struct dentry *dentry)
{
    return __d_entry_type(dentry) == DCACHE_DIRECTORY_TYPE;
}

#endif /* _LINUX_DCACHE_H */

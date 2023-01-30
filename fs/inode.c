// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <bug.h>
#include <hash.h>
#include <slab.h>
#include <errno.h>
#include <export.h>
#include <kernel.h>
#include <string.h>
#include <hashtable.h>

static unsigned int i_hash_mask;
static unsigned int i_hash_shift;
static struct hlist_head *inode_hashtable;

static struct kmem_cache *inode_cachep;

void
inode_init_once(struct inode *inode)
{
    memset(inode, 0, sizeof(*inode));
    /*
    INIT_HLIST_NODE(&inode->i_hash);
    INIT_LIST_HEAD(&inode->i_devices);
    INIT_LIST_HEAD(&inode->i_io_list);
    INIT_LIST_HEAD(&inode->i_wb_list);
    INIT_LIST_HEAD(&inode->i_lru);
    */
    //__address_space_init_once(&inode->i_data);
    //i_size_ordered_init(inode);
}
EXPORT_SYMBOL(inode_init_once);

static unsigned long
hash(struct super_block *sb, unsigned long hashval)
{
    unsigned long tmp;
    tmp = (hashval * (unsigned long)sb) ^
        (GOLDEN_RATIO_PRIME + hashval) / L1_CACHE_BYTES;
    tmp = tmp ^ ((tmp ^ GOLDEN_RATIO_PRIME) >> i_hash_shift);
    return tmp & i_hash_mask;
}

const struct address_space_operations empty_aops = {
};
EXPORT_SYMBOL(empty_aops);

int
inode_init_always(struct super_block *sb, struct inode *inode)
{
    struct address_space *const mapping = &inode->i_data;

    inode->i_sb = sb;
    inode->i_blkbits = sb->s_blocksize_bits;
    inode->i_mapping = mapping;
    INIT_HLIST_HEAD(&inode->i_dentry);  /* buggered by rcu freeing */

    mapping->a_ops = &empty_aops;
    mapping->host = inode;
    printk("%s: host(%lx)\n", __func__, inode);
    return 0;
}

static struct inode *
alloc_inode(struct super_block *sb)
{
    struct inode *inode;
    const struct super_operations *ops = sb->s_op;

    if (ops->alloc_inode)
        inode = ops->alloc_inode(sb);
    else
        inode = kmem_cache_alloc(inode_cachep, GFP_KERNEL);

    if (!inode)
        return NULL;

    BUG_ON(unlikely(inode_init_always(sb, inode)));
    return inode;
}

struct inode *
new_inode_pseudo(struct super_block *sb)
{
    struct inode *inode = alloc_inode(sb);

    if (inode) {
        inode->i_state = 0;
        INIT_LIST_HEAD(&inode->i_sb_list);
    }
    return inode;
}

void
inode_sb_list_add(struct inode *inode)
{
    list_add(&inode->i_sb_list, &inode->i_sb->s_inodes);
}
EXPORT_SYMBOL(inode_sb_list_add);

struct inode *
new_inode(struct super_block *sb)
{
    struct inode *inode;

    inode = new_inode_pseudo(sb);
    if (inode)
        inode_sb_list_add(inode);
    return inode;
}
EXPORT_SYMBOL(new_inode);

static struct inode *
find_inode(struct super_block *sb,
           struct hlist_head *head,
           int (*test)(struct inode *, void *),
           void *data)
{
    struct inode *inode = NULL;

repeat:
    hlist_for_each_entry(inode, head, i_hash) {
        if (inode->i_sb != sb)
            continue;
        if (!test(inode, data))
            continue;
        return inode;
    }
    return NULL;
}

struct inode *
ilookup5_nowait(struct super_block *sb, unsigned long hashval,
                int (*test)(struct inode *, void *), void *data)
{
    struct hlist_head *head = inode_hashtable + hash(sb, hashval);
    struct inode *inode = find_inode(sb, head, test, data);
    return IS_ERR(inode) ? NULL : inode;
}
EXPORT_SYMBOL(ilookup5_nowait);

struct inode *
ilookup5(struct super_block *sb, unsigned long hashval,
         int (*test)(struct inode *, void *), void *data)
{
    struct inode *inode;
again:
    inode = ilookup5_nowait(sb, hashval, test, data);
    if (inode) {
        if (unlikely(inode_unhashed(inode))) {
            iput(inode);
            goto again;
        }
    }
    return inode;
}
EXPORT_SYMBOL(ilookup5);

struct inode *
inode_insert5(struct inode *inode,
              unsigned long hashval,
              int (*test)(struct inode *, void *),
              int (*set)(struct inode *, void *),
              void *data)
{
    bool creating = inode->i_state & I_CREATING;
    struct hlist_head *head = inode_hashtable + hash(inode->i_sb, hashval);
    BUG_ON(find_inode(inode->i_sb, head, test, data));

    if (set && unlikely(set(inode, data))) {
        panic("bad set func!");
    }

    inode->i_state |= I_NEW;
    hlist_add_head(&inode->i_hash, head);
    if (!creating)
        inode_sb_list_add(inode);

    return inode;
}

static struct inode *
find_inode_fast(struct super_block *sb,
                struct hlist_head *head,
                unsigned long ino)
{
    struct inode *inode = NULL;

    hlist_for_each_entry(inode, head, i_hash) {
        if (inode->i_ino != ino)
            continue;
        if (inode->i_sb != sb)
            continue;
        if (unlikely(inode->i_state & I_CREATING))
            return ERR_PTR(-ESTALE);
        return inode;
    }
    return NULL;
}

struct inode *iget_locked(struct super_block *sb, unsigned long ino)
{
    struct inode *inode;
    struct hlist_head *head = inode_hashtable + hash(sb, ino);

 again:
    inode = find_inode_fast(sb, head, ino);
    if (inode) {
        if (IS_ERR(inode))
            return NULL;
        if (unlikely(inode_unhashed(inode))) {
            iput(inode);
            goto again;
        }
        return inode;
    }

    inode = alloc_inode(sb);
    if (inode) {
        struct inode *old = find_inode_fast(sb, head, ino);
        BUG_ON(old);

        inode->i_ino = ino;
        inode->i_state = I_NEW;
        hlist_add_head(&inode->i_hash, head);
        printk("%s: step2 (%p) (%p)\n",
               __func__,
               inode->i_sb_list.prev,
               inode->i_sb->s_inodes.prev);
        inode_sb_list_add(inode);
        printk("%s: step3\n", __func__);
    }
    return inode;
}
EXPORT_SYMBOL(iget_locked);

struct inode *
iget5_locked(struct super_block *sb, unsigned long hashval,
             int (*test)(struct inode *, void *),
             int (*set)(struct inode *, void *), void *data)
{
    struct inode *inode = ilookup5(sb, hashval, test, data);
    if (!inode) {
        struct inode *new = alloc_inode(sb);

        if (new) {
            new->i_state = 0;
            inode = inode_insert5(new, hashval, test, set, data);
            if (unlikely(inode != new))
                panic("bad inode!");
        }
    }
    return inode;
}
EXPORT_SYMBOL(iget5_locked);

static void
init_once(void *foo)
{
}

void
inode_init(void)
{
    /* inode slab cache */
    inode_cachep = kmem_cache_create("inode_cache",
                                     sizeof(struct inode),
                                     0,
                                     (SLAB_RECLAIM_ACCOUNT|SLAB_PANIC|
                                      SLAB_MEM_SPREAD|SLAB_ACCOUNT),
                                     init_once);

    inode_hashtable =
        alloc_large_system_hash("Inode-cache",
                                sizeof(struct hlist_head),
                                14,
                                &i_hash_shift,
                                &i_hash_mask);
}

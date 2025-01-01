/* SPDX-License-Identifier: GPL-2.0-only */

#include <fs.h>
#include <bug.h>
#include <slab.h>
#include <dcache.h>
#include <printk.h>
#include <export.h>
#include "internal.h"

bool procfs_ready = false;
EXPORT_SYMBOL(procfs_ready);

static struct kmem_cache *proc_inode_cachep;

/*
 * This is the root "inode" in the /proc tree..
 */
struct proc_dir_entry proc_root = {
    .name       = "/proc",
};

struct proc_inode {
    struct hlist_node sibling_inodes;
    struct inode vfs_inode;
};

struct proc_fs_context {
    unsigned int mask;
};

static struct inode *proc_alloc_inode(struct super_block *sb)
{
    struct proc_inode *ei;

    ei = kmem_cache_alloc(proc_inode_cachep, GFP_KERNEL);
    if (!ei)
        return NULL;
    INIT_HLIST_NODE(&ei->sibling_inodes);
    return &ei->vfs_inode;
}

const struct super_operations proc_sops = {
    .alloc_inode    = proc_alloc_inode,
};

static int proc_fill_super(struct super_block *s, struct fs_context *fc)
{
    struct inode *root_inode;

    /* User space would break if executables or devices appear on proc */
    s->s_flags |= SB_NODIRATIME | SB_NOSUID | SB_NOEXEC;
    s->s_blocksize = 1024;
    s->s_blocksize_bits = 10;
    s->s_op = &proc_sops;

    root_inode = proc_get_inode(s, &proc_root);
    if (!root_inode) {
        panic("proc_fill_super: get root inode failed");
        return -ENOMEM;
    }

    s->s_root = d_make_root(root_inode);
    if (!s->s_root) {
        panic("proc_fill_super: allocate dentry failed");
        return -ENOMEM;
    }
    return 0;
}

static int proc_get_tree(struct fs_context *fc)
{
    return get_tree_nodev(fc, proc_fill_super);
}

static const struct fs_context_operations proc_fs_context_ops = {
    .get_tree   = proc_get_tree,
};

static int proc_init_fs_context(struct fs_context *fc)
{
    struct proc_fs_context *ctx;

    ctx = kzalloc(sizeof(struct proc_fs_context), GFP_KERNEL);
    if (!ctx)
        return -ENOMEM;

    fc->fs_private = ctx;
    fc->ops = &proc_fs_context_ops;
    return 0;
}

static struct file_system_type proc_fs_type = {
    .name               = "proc",
    .init_fs_context    = proc_init_fs_context,
};

void proc_root_init(void)
{
    register_filesystem(&proc_fs_type);
}

static void init_once(void *foo)
{
    struct proc_inode *ei = (struct proc_inode *) foo;

    inode_init_once(&ei->vfs_inode);
}

void proc_init_kmemcache(void)
{
    proc_inode_cachep =
        kmem_cache_create("proc_inode_cache", sizeof(struct proc_inode),
                          0, (SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD|
                              SLAB_ACCOUNT|SLAB_PANIC), init_once);
}

int
init_module(void)
{
    printk("module[procfs]: init begin ...\n");

    proc_init_kmemcache();

    proc_root_init();

    procfs_ready = true;

    printk("module[procfs]: init end!\n");
    return 0;
}

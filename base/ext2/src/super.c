// SPDX-License-Identifier: GPL-2.0-only

#include <fs.h>
#include <slab.h>
#include <stat.h>
#include <errno.h>
#include <dcache.h>
#include <export.h>
#include <fs/ext2.h>
#include <buffer_head.h>

static struct kmem_cache *ext2_inode_cachep;

static struct inode *ext2_alloc_inode(struct super_block *sb)
{
    struct ext2_inode_info *ei;
    ei = kmem_cache_alloc(ext2_inode_cachep, GFP_KERNEL);
    if (!ei)
        return NULL;

    return &ei->vfs_inode;
}

static const struct super_operations ext2_sops = {
    .alloc_inode    = ext2_alloc_inode,
};

static unsigned long
get_sb_block(void **data)
{
    BUG_ON((*data));

    return 1;   /* Default location */
}

static unsigned long
descriptor_loc(struct super_block *sb,
               unsigned long logic_sb_block,
               int nr)
{
    return (logic_sb_block + nr + 1);
}

static int
ext2_fill_super(struct super_block *sb, void *data, int silent)
{
    int i, j;
    int db_count;
    struct inode *root;
    unsigned long block;
    struct buffer_head *bh;
    struct ext2_sb_info *sbi;
    struct ext2_super_block *es;
    unsigned long logic_sb_block;
    unsigned long offset = 0;
    int blocksize = BLOCK_SIZE;
    unsigned long sb_block = get_sb_block(&data);

    sbi = kzalloc(sizeof(*sbi), GFP_KERNEL);
    if (!sbi)
        panic("out of memory!");

    sb->s_fs_info = sbi;
    sbi->s_sb_block = sb_block;

    blocksize = sb_min_blocksize(sb, BLOCK_SIZE);
    if (!blocksize)
        panic("error: unable to set blocksize");

    if (blocksize != BLOCK_SIZE) {
        logic_sb_block = (sb_block*BLOCK_SIZE) / blocksize;
        offset = (sb_block*BLOCK_SIZE) % blocksize;
        printk("%s: 1 logic_sb_block(%lu, %lu)\n", __func__, logic_sb_block, BLOCK_SIZE);
    } else {
        logic_sb_block = sb_block;
        printk("%s: 2 logic_sb_block(%lu, %lu)\n", __func__, logic_sb_block, BLOCK_SIZE);
    }

    if (!(bh = sb_bread_unmovable(sb, logic_sb_block)))
        panic("error: unable to read superblock");

    es = (struct ext2_super_block *) (((char *)bh->b_data) + offset);
    sbi->s_es = es;

    blocksize = BLOCK_SIZE << sbi->s_es->s_log_block_size;

    if (sb->s_blocksize != blocksize) {
        if (!sb_set_blocksize(sb, blocksize))
            panic("bad blocksize %d", blocksize);

        logic_sb_block = (sb_block * BLOCK_SIZE) / blocksize;
        printk("%s: logic_sb_block(%lu) blocksize(%lu) sb_block(%lu)\n",
               __func__, logic_sb_block, blocksize, sb_block);
        offset = (sb_block*BLOCK_SIZE) % blocksize;
        bh = sb_bread_unmovable(sb, logic_sb_block);
        if(!bh)
            panic("couldn't readsuperblock on 2nd try");

        es = (struct ext2_super_block *) (((char *)bh->b_data) + offset);
        sbi->s_es = es;
        if (es->s_magic != EXT2_SUPER_MAGIC)
            panic("magic mismatch");
    }

    if (es->s_rev_level == EXT2_GOOD_OLD_REV) {
        panic("bad s_rev_level!");
    } else {
        sbi->s_inode_size = es->s_inode_size;
        if ((sbi->s_inode_size < EXT2_GOOD_OLD_INODE_SIZE) ||
            !is_power_of_2(sbi->s_inode_size) ||
            (sbi->s_inode_size > blocksize)) {
            panic("unsupported inode size: %d", sbi->s_inode_size);
        }
    }

    sbi->s_inodes_per_group = es->s_inodes_per_group;
    sbi->s_blocks_per_group = es->s_blocks_per_group;
    sbi->s_desc_per_block = sb->s_blocksize
        / sizeof (struct ext2_group_desc);
    sbi->s_desc_per_block_bits = ilog2(EXT2_DESC_PER_BLOCK(sb));
    sbi->s_addr_per_block_bits = ilog2(EXT2_ADDR_PER_BLOCK(sb));

    if (EXT2_BLOCKS_PER_GROUP(sb) == 0)
        panic("can not find blocks per group!");

    sbi->s_groups_count =
        ((es->s_blocks_count - es->s_first_data_block - 1)
         / EXT2_BLOCKS_PER_GROUP(sb)) + 1;

    db_count = (sbi->s_groups_count + EXT2_DESC_PER_BLOCK(sb) - 1) /
        EXT2_DESC_PER_BLOCK(sb);

    sbi->s_group_desc = kmalloc_array(db_count,
                                      sizeof(struct buffer_head *),
                                      GFP_KERNEL);
    if (sbi->s_group_desc == NULL)
        panic("no memory!");

    for (i = 0; i < db_count; i++) {
        block = descriptor_loc(sb, logic_sb_block, i);
        sbi->s_group_desc[i] = sb_bread_unmovable(sb, block);
        if (!sbi->s_group_desc[i])
            panic("unable to read group descriptors");
    }

    sb->s_op = &ext2_sops;

    root = ext2_iget(sb, EXT2_ROOT_INO);
    if (IS_ERR(root))
        panic("can not get root inode! (%d)!", PTR_ERR(root));

    printk("%s: isdir(%d) i_blocks(%lu) i_size(%lu)\n",
           __func__, S_ISDIR(root->i_mode), root->i_blocks, root->i_size);

    if (!S_ISDIR(root->i_mode) || !root->i_blocks || !root->i_size)
        panic("error: corrupt root inode, run e2fsck");

    sb->s_root = d_make_root(root);
    if (!sb->s_root)
        panic("no root!");

    /*
    if (ext2_setup_super (sb, es, sb_rdonly(sb)))
        sb->s_flags |= SB_RDONLY;
    ext2_write_super(sb);
    */

    return 0;
}

static struct dentry *
ext2_mount(struct file_system_type *fs_type, int flags, const char *dev_name)
{
    return mount_bdev(fs_type, flags, dev_name, ext2_fill_super);
}

static struct file_system_type ext2_fs_type = {
    .name       = "ext2",
    .mount      = ext2_mount,
    .fs_flags   = FS_REQUIRES_DEV,
};

static void init_once(void *foo)
{
    struct ext2_inode_info *ei = (struct ext2_inode_info *) foo;

    inode_init_once(&ei->vfs_inode);
}

static int init_inodecache(void)
{
    ext2_inode_cachep =
        kmem_cache_create_usercopy("ext2_inode_cache",
                                   sizeof(struct ext2_inode_info), 0,
                                   (SLAB_RECLAIM_ACCOUNT|
                                    SLAB_MEM_SPREAD|
                                    SLAB_ACCOUNT),
                                   offsetof(struct ext2_inode_info, i_data),
                                   sizeof_field(struct ext2_inode_info, i_data),
                                   init_once);
    if (ext2_inode_cachep == NULL)
        return -ENOMEM;

    return 0;
}

int init_ext2_fs(void)
{
    int err;

    err = init_inodecache();
    if (err)
        return err;

    err = register_filesystem(&ext2_fs_type);
    if (err)
        panic("can not register ext2 fs!");

    return 0;
}

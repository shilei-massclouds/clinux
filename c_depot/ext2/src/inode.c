// SPDX-License-Identifier: GPL-2.0

#include <fs.h>
#include <stat.h>
#include <errno.h>
#include <kernel.h>
#include <fs/ext2.h>
#include <buffer_head.h>

typedef struct {
    u32 *p;
    u32 key;
    struct buffer_head *bh;
} Indirect;

static struct ext2_inode *
ext2_get_inode(struct super_block *sb, ino_t ino, struct buffer_head **p)
{
    unsigned long block;
    unsigned long offset;
    unsigned long block_group;
    struct buffer_head *bh;
    struct ext2_group_desc *gdp;

    block_group = (ino - 1) / EXT2_INODES_PER_GROUP(sb);
    gdp = ext2_get_group_desc(sb, block_group, NULL);
    if (!gdp)
        panic("bad group desc!");

    /*
     * Figure out the offset within the block group inode table
     */
    offset = ((ino - 1) % EXT2_INODES_PER_GROUP(sb)) * EXT2_INODE_SIZE(sb);
    block = gdp->bg_inode_table + (offset >> EXT2_BLOCK_SIZE_BITS(sb));
    if (!(bh = sb_bread_unmovable(sb, block)))
        panic("bad io!");

    printk("%s: ino(%lu) bg_inode_table(%u) offset(%lu) block(%lu)\n",
           __func__, ino, gdp->bg_inode_table, offset, block);

    *p = bh;
    offset &= (EXT2_BLOCK_SIZE(sb) - 1);
    return (struct ext2_inode *) (bh->b_data + offset);
}

static int
ext2_block_to_path(struct inode *inode,
                   long i_block, int offsets[4], int *boundary)
{
    int n = 0;
    int final = 0;
    int ptrs = EXT2_ADDR_PER_BLOCK(inode->i_sb);
    int ptrs_bits = EXT2_ADDR_PER_BLOCK_BITS(inode->i_sb);
    const long direct_blocks = EXT2_NDIR_BLOCKS;
    const long indirect_blocks = ptrs;
    const long double_blocks = (1 << (ptrs_bits * 2));

    if (i_block < 0) {
        panic("i_block < 0");
    } else if (i_block < direct_blocks) {
        offsets[n++] = i_block;
        final = direct_blocks;
    } else if ( (i_block -= direct_blocks) < indirect_blocks) {
        offsets[n++] = EXT2_IND_BLOCK;
        offsets[n++] = i_block;
        final = ptrs;
    } else if ((i_block -= indirect_blocks) < double_blocks) {
        offsets[n++] = EXT2_DIND_BLOCK;
        offsets[n++] = i_block >> ptrs_bits;
        offsets[n++] = i_block & (ptrs - 1);
        final = ptrs;
    } else if (((i_block -= double_blocks) >> (ptrs_bits * 2)) < ptrs) {
        offsets[n++] = EXT2_TIND_BLOCK;
        offsets[n++] = i_block >> (ptrs_bits * 2);
        offsets[n++] = (i_block >> ptrs_bits) & (ptrs - 1);
        offsets[n++] = i_block & (ptrs - 1);
        final = ptrs;
    } else {
        panic("i_block(%lu) is too big", i_block);
    }

    if (boundary)
        *boundary = final - 1 - (i_block & (ptrs - 1));

    return n;
}

static inline void
add_chain(Indirect *p, struct buffer_head *bh, u32 *v)
{
    p->key = *(p->p = v);
    p->bh = bh;
}

static inline int verify_chain(Indirect *from, Indirect *to)
{
    while (from <= to && from->key == *from->p)
        from++;
    return (from > to);
}

static Indirect *
ext2_get_branch(struct inode *inode, int depth, int *offsets,
                Indirect chain[4], int *err)
{
    struct buffer_head *bh;
    Indirect *p = chain;
    struct super_block *sb = inode->i_sb;

    *err = 0;
    /* i_data is not going away, no lock needed */
    add_chain(chain, NULL, EXT2_I(inode)->i_data + *offsets);
    printk("%s 1: key(%x) %x\n", __func__, p->key, *(EXT2_I(inode)->i_data));
    if (!p->key)
        panic("no block!");
    while (--depth) {
        bh = sb_bread(sb, p->key);
        if (!bh)
            panic("bread error!");
        if (!verify_chain(chain, p))
            panic("changed!");
        add_chain(++p, bh, (u32*)bh->b_data + *++offsets);
        if (!p->key)
            panic("no block!");
    }
    return NULL;
}

static int
ext2_get_blocks(struct inode *inode,
                sector_t iblock, unsigned long maxblocks,
                u32 *bno, bool *new, bool *boundary, int create)
{
    int err;
    int offsets[4];
    Indirect chain[4];
    Indirect *partial;
    ext2_fsblk_t goal;
    int indirect_blks;
    int blocks_to_boundary = 0;
    int depth;
    struct ext2_inode_info *ei = EXT2_I(inode);
    int count = 0;
    ext2_fsblk_t first_block = 0;

    BUG_ON(maxblocks == 0);

    depth = ext2_block_to_path(inode, iblock, offsets,
                               &blocks_to_boundary);

    if (depth == 0)
        return -EIO;

    partial = ext2_get_branch(inode, depth, offsets, chain, &err);
    /* Simplest case - block found, no allocation needed */
    if (!partial) {
        first_block = chain[depth - 1].key;
        count++;
        /*map more blocks*/
        while (count < maxblocks && count <= blocks_to_boundary) {
            ext2_fsblk_t blk;

            if (!verify_chain(chain, chain + depth - 1)) {
                /*
                 * Indirect block might be removed by
                 * truncate while we were reading it.
                 * Handling of that case: forget what we've
                 * got now, go to reread.
                 */
                err = -EAGAIN;
                count = 0;
                partial = chain + depth - 1;
                break;
            }
            blk = *(chain[depth-1].p + count);
            if (blk == first_block + count)
                count++;
            else
                break;
        }

        if (err != -EAGAIN)
            goto got_it;
    }

    panic("%s: !", __func__);

got_it:
    if (count > blocks_to_boundary)
        *boundary = true;
    err = count;
    /* Clean up and exit */
    partial = chain + depth - 1;    /* the whole chain */
cleanup:
    while (partial > chain)
        partial--;
    if (err > 0)
        *bno = chain[depth-1].key;
    return err;
}

int ext2_get_block(struct inode *inode, sector_t iblock,
                   struct buffer_head *bh_result, int create)
{
    int ret;
    u32 bno;
    unsigned max_blocks = bh_result->b_size >> inode->i_blkbits;
    bool new = false, boundary = false;

    ret = ext2_get_blocks(inode, iblock, max_blocks, &bno,
                          &new, &boundary, create);
    if (ret <= 0)
        return ret;

    map_bh(bh_result, inode->i_sb, bno);
    bh_result->b_size = (ret << inode->i_blkbits);
    if (new)
        set_buffer_new(bh_result);
    if (boundary)
        set_buffer_boundary(bh_result);
    return 0;
}

static int ext2_readpage(struct file *file, struct page *page)
{
    return mpage_readpage(page, ext2_get_block);
}

static void ext2_readahead(struct readahead_control *rac)
{
    mpage_readahead(rac, ext2_get_block);
}

const struct address_space_operations ext2_aops = {
    .readpage   = ext2_readpage,
    .readahead  = ext2_readahead,
};

void ext2_set_file_ops(struct inode *inode)
{
    inode->i_op = &ext2_file_inode_operations;
    inode->i_fop = &ext2_file_operations;
    inode->i_mapping->a_ops = &ext2_aops;
}

struct inode *ext2_iget(struct super_block *sb, unsigned long ino)
{
    int n;
    struct inode *inode;
    struct ext2_inode_info *ei;
    struct ext2_inode *raw_inode;
    struct buffer_head *bh = NULL;

    inode = iget_locked(sb, ino);
    if (!inode)
        return ERR_PTR(-ENOMEM);
    if (!(inode->i_state & I_NEW))
        return inode;

    ei = EXT2_I(inode);

    raw_inode = ext2_get_inode(inode->i_sb, ino, &bh);
    if (IS_ERR(raw_inode))
        panic("bad inode!");

    printk("%s: i_block0(%x)\n", __func__, raw_inode->i_block[0]);

    inode->i_mode = raw_inode->i_mode;
    inode->i_size = raw_inode->i_size;
    inode->i_blocks = raw_inode->i_blocks;

    /*
     * NOTE! The in-memory inode i_data array is in little-endian order
     * even on big-endian machines: we do NOT byteswap the block numbers!
     */
    for (n = 0; n < EXT2_N_BLOCKS; n++)
        ei->i_data[n] = raw_inode->i_block[n];

    if (S_ISREG(inode->i_mode)) {
        ext2_set_file_ops(inode);
    } else if (S_ISDIR(inode->i_mode)) {
        printk("%s: ino(%lu)\n", __func__, ino);
        inode->i_op = &ext2_dir_inode_operations;
        inode->i_mapping->a_ops = &ext2_aops;
    } else {
        panic("unknown file!");
    }

    return inode;
}

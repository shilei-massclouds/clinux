/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _EXT2_H_
#define _EXT2_H_

#include <fs.h>
#include <types.h>
#include <dcache.h>
#include <buffer_head.h>

#define EXT2_SUPER_MAGIC    0xEF53

#define EXT2_NAME_LEN 255

/*
 * Special inode numbers
 */
#define EXT2_BAD_INO    1  /* Bad blocks inode */
#define EXT2_ROOT_INO   2  /* Root inode */

/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS    12
#define EXT2_IND_BLOCK  EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS   (EXT2_TIND_BLOCK + 1)

#define EXT2_GOOD_OLD_REV   0   /* The good old (original) format */
#define EXT2_GOOD_OLD_INODE_SIZE 128

#define EXT2_DIR_PAD    4
#define EXT2_DIR_ROUND  (EXT2_DIR_PAD - 1)
#define EXT2_DIR_REC_LEN(name_len) \
    (((name_len) + 8 + EXT2_DIR_ROUND) & ~EXT2_DIR_ROUND)

#define EXT2_ADDR_PER_BLOCK_BITS(s) (EXT2_SB(s)->s_addr_per_block_bits)

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry_2 {
    u32     inode;          /* Inode number */
    u16     rec_len;        /* Directory entry length */
    u8      name_len;       /* Name length */
    u8      file_type;
    char    name[];         /* File name, up to EXT2_NAME_LEN */
};

/*
 * Structure of the super block
 */
struct ext2_super_block {
	u32	s_inodes_count;		/* Inodes count */
	u32	s_blocks_count;		/* Blocks count */
	u32	s_r_blocks_count;	/* Reserved blocks count */
	u32	s_free_blocks_count;	/* Free blocks count */
	u32	s_free_inodes_count;	/* Free inodes count */
	u32	s_first_data_block;	/* First Data Block */
	u32	s_log_block_size;	/* Block size */
	u32	s_log_frag_size;	/* Fragment size */
	u32	s_blocks_per_group;	/* # Blocks per group */
	u32	s_frags_per_group;	/* # Fragments per group */
	u32	s_inodes_per_group;	/* # Inodes per group */
	u32	s_mtime;		/* Mount time */
	u32	s_wtime;		/* Write time */
	u16	s_mnt_count;		/* Mount count */
	u16	s_max_mnt_count;	/* Maximal mount count */
	u16	s_magic;		/* Magic signature */
	u16	s_state;		/* File system state */
	u16	s_errors;		/* Behaviour when detecting errors */
	u16	s_minor_rev_level;  /* minor revision level */
	u32	s_lastcheck;		/* time of last check */
	u32	s_checkinterval;	/* max. time between checks */
	u32	s_creator_os;		/* OS */
	u32	s_rev_level;		/* Revision level */
	u16	s_def_resuid;		/* Default uid for reserved blocks */
	u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32	s_first_ino;        /* First non-reserved inode */
	u16 s_inode_size;       /* size of inode structure */
	u16	s_block_group_nr;   /* block group # of this superblock */
	u32	s_feature_compat;   /* compatible feature set */
	u32	s_feature_incompat; /* incompatible feature set */
	u32	s_feature_ro_compat;    /* readonly-compatible feature set */
	__u8	s_uuid[16];		    /* 128-bit uuid for volume */
	char	s_volume_name[16];  /* volume name */
	char	s_last_mounted[64]; /* directory where last mounted */
	u32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	__u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	__u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	__u16	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	__u8	s_journal_uuid[16];	/* uuid of journal superblock */
	__u32	s_journal_inum;		/* inode number of journal file */
	__u32	s_journal_dev;		/* device number of journal file */
	__u32	s_last_orphan;		/* start of list of inodes to delete */
	__u32	s_hash_seed[4];		/* HTREE hash seed */
	__u8	s_def_hash_version;	/* Default hash version to use */
	__u8	s_reserved_char_pad;
	__u16	s_reserved_word_pad;
	u32	s_default_mount_opts;
    u32	s_first_meta_bg;        /* First metablock block group */
	__u32	s_reserved[190];	/* Padding to the end of the block */
};

struct ext2_inode_info {
    u32 i_data[15];

    u32 i_dir_start_lookup;

    struct inode vfs_inode;
};

struct ext2_sb_info {
    unsigned long s_sb_block;
    unsigned long s_inodes_per_group;   /* #Inodes in a group */
    unsigned long s_blocks_per_group;   /* Number of blocks in a group */
    unsigned long s_desc_per_block;     /* #group-desc per block */
    unsigned long s_groups_count;       /* Number of groups in the fs */
    struct ext2_super_block *s_es;      /* To super block in the buf */
    int s_desc_per_block_bits;
    int s_inode_size;
    struct buffer_head ** s_group_desc;
    int s_addr_per_block_bits;
};

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
    u32  bg_block_bitmap;       /* Blocks bitmap block */
    u32  bg_inode_bitmap;       /* Inodes bitmap block */
    u32  bg_inode_table;        /* Inodes table block */
    u16  bg_free_blocks_count;  /* Free blocks count */
    u16  bg_free_inodes_count;  /* Free inodes count */
    u16  bg_used_dirs_count;    /* Directories count */
    u16  bg_pad;
    u32  bg_reserved[3];
};

/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
    u16  i_mode;     /* File mode */
    u16  i_uid;      /* Low 16 bits of Owner Uid */
    u32  i_size;     /* Size in bytes */
    u32  i_atime;    /* Access time */
    u32  i_ctime;    /* Creation time */
    u32  i_mtime;    /* Modification time */
    u32  i_dtime;    /* Deletion Time */
    u16  i_gid;      /* Low 16 bits of Group Id */
    u16  i_links_count;  /* Links count */
    u32  i_blocks;   /* Blocks count */
    u32  i_flags;    /* File flags */
    union {
        struct {
            u32  l_i_reserved1;
        } linux1;
        struct {
            u32  h_i_translator;
        } hurd1;
        struct {
            u32  m_i_reserved1;
        } masix1;
    } osd1;             /* OS dependent 1 */
    u32  i_block[EXT2_N_BLOCKS];/* Pointers to blocks */
    u32  i_generation;   /* File version (for NFS) */
    u32  i_file_acl; /* File ACL */
    u32  i_dir_acl;  /* Directory ACL */
    u32  i_faddr;    /* Fragment address */
    union {
        struct {
            __u8    l_i_frag;   /* Fragment number */
            __u8    l_i_fsize;  /* Fragment size */
            __u16   i_pad1;
            u16  l_i_uid_high;   /* these 2 fields    */
            u16  l_i_gid_high;   /* were reserved2[0] */
            __u32   l_i_reserved2;
        } linux2;
        struct {
            __u8    h_i_frag;   /* Fragment number */
            __u8    h_i_fsize;  /* Fragment size */
            u16  h_i_mode_high;
            u16  h_i_uid_high;
            u16  h_i_gid_high;
            u32  h_i_author;
        } hurd2;
        struct {
            __u8    m_i_frag;   /* Fragment number */
            __u8    m_i_fsize;  /* Fragment size */
            __u16   m_pad1;
            __u32   m_i_reserved2[2];
        } masix2;
    } osd2;             /* OS dependent 2 */
};

extern const struct inode_operations ext2_dir_inode_operations;

/* data type for filesystem-wide blocks number */
typedef unsigned long ext2_fsblk_t;

/*
 * Macro-instructions used to manage group descriptors
 */
#define EXT2_BLOCKS_PER_GROUP(s)    (EXT2_SB(s)->s_blocks_per_group)
#define EXT2_INODES_PER_GROUP(s)    (EXT2_SB(s)->s_inodes_per_group)
#define EXT2_DESC_PER_BLOCK(s)      (EXT2_SB(s)->s_desc_per_block)
#define EXT2_DESC_PER_BLOCK_BITS(s) (EXT2_SB(s)->s_desc_per_block_bits)

#define EXT2_INODE_SIZE(s)          (EXT2_SB(s)->s_inode_size)

#define EXT2_BLOCK_SIZE(s)      ((s)->s_blocksize)
#define EXT2_BLOCK_SIZE_BITS(s) ((s)->s_blocksize_bits)
#define EXT2_ADDR_PER_BLOCK(s)  (EXT2_BLOCK_SIZE(s) / sizeof (__u32))

static inline struct ext2_inode_info *EXT2_I(struct inode *inode)
{
    return container_of(inode, struct ext2_inode_info, vfs_inode);
}

struct inode *ext2_iget(struct super_block *sb, unsigned long ino);

static inline struct ext2_sb_info *EXT2_SB(struct super_block *sb)
{
    return sb->s_fs_info;
}

struct ext2_group_desc *
ext2_get_group_desc(struct super_block * sb,
                    unsigned int block_group,
                    struct buffer_head **bh);

int ext2_inode_by_name(struct inode *dir, const struct qstr *child,
                       ino_t *ino);

extern const struct inode_operations ext2_file_inode_operations;
extern const struct file_operations ext2_file_operations;

#endif /* _EXT2_H_ */

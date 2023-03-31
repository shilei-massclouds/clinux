// SPDX-License-Identifier: GPL-2.0

#include <errno.h>
#include <dcache.h>
#include <fs/ext2.h>
#include <pagemap.h>
#include <highmem.h>

typedef struct ext2_dir_entry_2 ext2_dirent;

static struct page *
ext2_get_page(struct inode *dir, unsigned long n, int quiet)
{
    struct address_space *mapping = dir->i_mapping;
    struct page *page = read_mapping_page(mapping, n, NULL);
    if (!IS_ERR(page)) {
        kmap(page);
    }
    return page;
}

/*
 * Return the offset into page `page_nr' of the last valid
 * byte in that page, plus one.
 */
static unsigned
ext2_last_byte(struct inode *inode, unsigned long page_nr)
{
    unsigned last_byte = inode->i_size;

    last_byte -= page_nr << PAGE_SHIFT;
    if (last_byte > PAGE_SIZE)
        last_byte = PAGE_SIZE;
    return last_byte;
}

/*
 * NOTE! unlike strncmp, ext2_match returns 1 for success, 0 for failure.
 *
 * len <= EXT2_NAME_LEN and de != NULL are guaranteed by caller.
 */
static inline int
ext2_match(int len, const char * const name,
           struct ext2_dir_entry_2 *de)
{
    if (len != de->name_len)
        return 0;
    if (!de->inode)
        return 0;
    return !memcmp(name, de->name, len);
}

/*
 * p is at least 6 bytes before the end of page
 */
static inline ext2_dirent *ext2_next_entry(ext2_dirent *p)
{
    return (ext2_dirent *)((char *)p + p->rec_len);
}

/*
 *  ext2_find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the page in which the entry was found (as a parameter - res_page),
 * and the entry itself. Page is returned mapped and unlocked.
 * Entry is guaranteed to be valid.
 */
struct ext2_dir_entry_2 *
ext2_find_entry(struct inode *dir, const struct qstr *child,
                struct page **res_page)
{
    ext2_dirent *de;
    unsigned long start, n;
    struct page *page = NULL;
    unsigned long npages = dir_pages(dir);
    struct ext2_inode_info *ei = EXT2_I(dir);
    const char *name = child->name;
    int namelen = child->len;
    unsigned reclen = EXT2_DIR_REC_LEN(namelen);

    if (npages == 0)
        return ERR_PTR(-ENOENT);

    /* OFFSET_CACHE */
    *res_page = NULL;

    start = ei->i_dir_start_lookup;
    if (start >= npages)
        start = 0;
    n = start;
    do {
        char *kaddr;
        page = ext2_get_page(dir, n, 0);
        if (IS_ERR(page))
            panic("bad page!");

        kaddr = page_address(page);
        de = (ext2_dirent *) kaddr;
        kaddr += ext2_last_byte(dir, n) - reclen;

        while ((char *)de <= kaddr) {
            if (de->rec_len == 0)
                panic("zero-length directory entry");

            printk("%s: name(%s, %s)\n", __func__, name, de->name);
            if (ext2_match(namelen, name, de))
                goto found;
            de = ext2_next_entry(de);
        }

        if (++n >= npages)
            n = 0;

        /* next page is past the blocks we've got */
        if (unlikely(n > (dir->i_blocks >> (PAGE_SHIFT - 9))))
            panic("dir %lu size %ld exceeds block count %lu",
                  dir->i_ino, (long)dir->i_size,
                  (unsigned long)dir->i_blocks);

    } while(n != start);

    return ERR_PTR(-ENOENT);

 found:
    *res_page = page;
    ei->i_dir_start_lookup = n;
    return de;
}

int ext2_inode_by_name(struct inode *dir, const struct qstr *child,
                       ino_t *ino)
{
    struct page *page;
    struct ext2_dir_entry_2 *de;

    de = ext2_find_entry(dir, child, &page);
    if (IS_ERR(de))
        return PTR_ERR(de);

    *ino = de->inode;
    return 0;
}

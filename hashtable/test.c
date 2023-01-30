// SPDX-License-Identifier: GPL-2.0-only

#include <printk.h>
#include <hashtable.h>

static void
test_hash_table(void)
{
    struct hlist_head *tbl;
    unsigned int i_hash_mask;
    unsigned int i_hash_shift;

    printk(_GREEN("Test hash table ...\n"));
    tbl = alloc_large_system_hash("Inode-cache",
                                  sizeof(struct hlist_head),
                                  14,
                                  &i_hash_shift,
                                  &i_hash_mask);

    printk(_GREEN("Test hash table ok!\n"));
}

static int
init_module(void)
{
    printk("module[test_rbtree]: init begin ...\n");

    test_hash_table();

    printk("module[test_rbtree]: init end!\n");

    return 0;
}

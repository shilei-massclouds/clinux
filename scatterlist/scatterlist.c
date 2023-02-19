// SPDX-License-Identifier: GPL-2.0-only

#include <export.h>
#include <printk.h>
#include <string.h>
#include <scatterlist.h>

/**
 * sg_init_table - Initialize SG table
 * @sgl:       The SG table
 * @nents:     Number of entries in table
 *
 * Notes:
 *   If this is part of a chained sg table, sg_mark_end() should be
 *   used only on the last table part.
 *
 **/
void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
    memset(sgl, 0, sizeof(*sgl) * nents);
    sg_init_marker(sgl, nents);
}
EXPORT_SYMBOL(sg_init_table);

/**
 * sg_next - return the next scatterlist entry in a list
 * @sg:     The current sg entry
 *
 * Description:
 *   Usually the next entry will be @sg@ + 1, but if this sg element is part
 *   of a chained scatterlist, it could jump to the start of a new
 *   scatterlist array.
 *
 **/
struct scatterlist *sg_next(struct scatterlist *sg)
{
    if (sg_is_last(sg))
        return NULL;

    sg++;
    if (unlikely(sg_is_chain(sg)))
        sg = sg_chain_ptr(sg);

    return sg;
}
EXPORT_SYMBOL(sg_next);

/**
 * sg_init_one - Initialize a single entry sg list
 * @sg:      SG entry
 * @buf:     Virtual address for IO
 * @buflen:  IO length
 *
 **/
void
sg_init_one(struct scatterlist *sg,
            const void *buf, unsigned int buflen)
{
    sg_init_table(sg, 1);
    sg_set_buf(sg, buf, buflen);
}
EXPORT_SYMBOL(sg_init_one);

int
init_module(void)
{
    printk("module[scatterlist]: init begin ...\n");
    printk("module[scatterlist]: init end!\n");
    return 0;
}

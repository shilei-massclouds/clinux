/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCATTERLIST_H
#define _LINUX_SCATTERLIST_H

#include <mm.h>
#include <bug.h>
#include <page.h>

#define SG_CHAIN    0x01UL
#define SG_END      0x02UL

#define sg_is_chain(sg) ((sg)->page_link & SG_CHAIN)
#define sg_is_last(sg)  ((sg)->page_link & SG_END)
#define sg_chain_ptr(sg) \
    ((struct scatterlist *) ((sg)->page_link & ~(SG_CHAIN | SG_END)))

struct scatterlist {
    unsigned long   page_link;
    unsigned int    offset;
    unsigned int    length;
    /*
    dma_addr_t      dma_address;
    */
};

void sg_init_table(struct scatterlist *sgl, unsigned int nents);

static inline void sg_mark_end(struct scatterlist *sg)
{
    /*
     * Set termination bit, clear potential chain bit
     */
    sg->page_link |= SG_END;
    sg->page_link &= ~SG_CHAIN;
}

static inline void
sg_init_marker(struct scatterlist *sgl, unsigned int nents)
{
    sg_mark_end(&sgl[nents - 1]);
}

static inline void sg_unmark_end(struct scatterlist *sg)
{
    sg->page_link &= ~SG_END;
}

struct scatterlist *sg_next(struct scatterlist *sg);

static inline void
sg_assign_page(struct scatterlist *sg, struct page *page)
{
    unsigned long page_link = sg->page_link & (SG_CHAIN | SG_END);

    /*
     * In order for the low bit stealing approach to work, pages
     * must be aligned at a 32-bit boundary as a minimum.
     */
    BUG_ON((unsigned long) page & (SG_CHAIN | SG_END));
    sg->page_link = page_link | (unsigned long) page;
}

/**
 * sg_set_page - Set sg entry to point at given page
 * @sg:      SG entry
 * @page:    The page
 * @len:     Length of data
 * @offset:  Offset into page
 *
 * Description:
 *   Use this function to set an sg entry pointing at a page, never assign
 *   the page directly. We encode sg table information in the lower bits
 *   of the page pointer. See sg_page() for looking up the page belonging
 *   to an sg entry.
 *
 **/
static inline void
sg_set_page(struct scatterlist *sg, struct page *page,
            unsigned int len, unsigned int offset)
{
    sg_assign_page(sg, page);
    sg->offset = offset;
    sg->length = len;
}

void
sg_init_one(struct scatterlist *sg,
            const void *buf, unsigned int buflen);

/**
 * sg_set_buf - Set sg entry to point at given data
 * @sg:      SG entry
 * @buf:     Data
 * @buflen:  Data length
 *
 **/
static inline void
sg_set_buf(struct scatterlist *sg, const void *buf, unsigned int buflen)
{
    sg_set_page(sg, virt_to_page(buf), buflen, offset_in_page(buf));
}

static inline struct page *sg_page(struct scatterlist *sg)
{
    return (struct page *)((sg)->page_link & ~(SG_CHAIN | SG_END));
}

/**
 * sg_phys - Return physical address of an sg entry
 * @sg:      SG entry
 *
 * Description:
 *   This calls page_to_phys() on the page in this sg entry, and adds the
 *   sg offset. The caller must know that it is legal to call page_to_phys()
 *   on the sg page.
 *
 **/
static inline dma_addr_t sg_phys(struct scatterlist *sg)
{
    return page_to_phys(sg_page(sg)) + sg->offset;
}

#endif /* _LINUX_SCATTERLIST_H */
